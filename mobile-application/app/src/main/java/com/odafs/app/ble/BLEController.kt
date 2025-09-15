package com.odafs.app.ble

import android.Manifest
import android.Manifest.permission.BLUETOOTH_SCAN
import android.annotation.SuppressLint
import android.app.Application
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothDevice.TRANSPORT_LE
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothStatusCodes
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Build
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.withTimeout
import kotlin.coroutines.cancellation.CancellationException

const val DEVICE_NAME = "odafs"
const val SERVICE_UUID = "9b19df40-4042-4479-0000-131cd24590be"
const val CHAR_UUID = "9b19df40-4042-4479-0001-131cd24590be"

object COMMANDS {
    const val HEALTH_CHECK = "health_check"
    const val AUDIO_HEALTH_CHECK = "audio_health_check"
    const val START_DISCOVERY = "start_discovery"
    const val STOP_DISCOVERY = "stop_discovery"
    const val GET_DEVICES = "get_devices"
    const val PAIR_DEVICE = "pair_device"
    const val CONNECT_DEVICE = "connect_device"
    const val DISCONNECT_DEVICE = "disconnect_device"
}


data class BLEDevice(
    val device: BluetoothDevice,
    val serviceUUIDs: List<ParcelUuid>
)

data class BTDevice(
    val name: String,
    val address: String
)

object BLEController {
    private var appContext: Application? = null

    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bleScanner: BluetoothLeScanner? = null

    private val seenAddresses = mutableSetOf<String>()
    private val _foundDevices = MutableStateFlow<List<BLEDevice>>(emptyList())
    val foundDevices: StateFlow<List<BLEDevice>> = _foundDevices.asStateFlow()

    private val _gattConnection = MutableStateFlow<BluetoothGatt?>(null)
    private val _serviceConnection = MutableStateFlow<BluetoothGattService?>(null)
    private val _characteristicConnection = MutableStateFlow<BluetoothGattCharacteristic?>(null)

    private val _connecting = MutableStateFlow(false)
    val connecting: StateFlow<Boolean> = _connecting.asStateFlow()

    private val _connected = MutableStateFlow(false)
    val connected: StateFlow<Boolean> = _connected.asStateFlow()

    private val _discovering = MutableStateFlow(false)
    val discovering: StateFlow<Boolean> = _discovering.asStateFlow()

    fun init(context: Context) {
        appContext = context.applicationContext as Application

        val manager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = manager.adapter

        bleScanner = bluetoothAdapter?.bluetoothLeScanner
    }

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            result.let {
                val device = it?.device
                val serviceUUIDs = it?.scanRecord?.serviceUuids

                if (device != null && serviceUUIDs != null) {
                    if (seenAddresses.add(device.address)) {
                        val bleDevice = BLEDevice(device, serviceUUIDs)
                        _foundDevices.value = _foundDevices.value + bleDevice
                    }
                }
            }
        }

        @SuppressLint("MissingPermission")
        override fun onBatchScanResults(results: MutableList<ScanResult>?) {
            Log.d("BLE Controller", "BLE Batch Scan Results: $results")
            results?.forEach { device ->
                device.let {
                    val device = it.device
                    val serviceUUIDs = it.scanRecord?.serviceUuids

                    if (device != null && serviceUUIDs != null) {
                        if (seenAddresses.add(device.address)) {
                            val bleDevice = BLEDevice(device, serviceUUIDs)
                            _foundDevices.value = _foundDevices.value + bleDevice
                        }
                    }
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("BLE", "Scan failed: $errorCode")
        }
    }

    private var pendingTransaction: CompletableDeferred<ByteArray?>? = null

    private var failedHealthChecks = 0
    private var failedAudioHealthChecks = 0
    private var failedHealthChecksThreshold = 8

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun BluetoothGatt.writeAndReadCharacteristic(
        characteristic: BluetoothGattCharacteristic,
        value: ByteArray
    ) : ByteArray? {
        if (pendingTransaction != null) {
            Log.e("BLE Controller", "Another transaction is pending")
            return null
        }

        val pending = CompletableDeferred<ByteArray?>()
        pendingTransaction = pending

        val startedTransaction = writeCharacteristic(
            characteristic,
            value,
            BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        )

        if (startedTransaction != BluetoothStatusCodes.SUCCESS) {
            pending.completeExceptionally(Exception("Characteristic write failed"))
            pendingTransaction = null
            return null
        }

        try {
            return withTimeout(20000) {
                pendingTransaction?.await()
            }
        }
        catch (e: CancellationException) {
            pendingTransaction?.cancel()
            pendingTransaction = null
            throw e
        }
        catch (e: Throwable) {
            pendingTransaction?.completeExceptionally(e)
            pendingTransaction = null
            throw e
        }
        finally {
            pendingTransaction = null
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothGatt.STATE_CONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Connected")
                    _gattConnection.value = gatt
                    gatt.discoverServices()
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Disconnected")
                    disconnectFromDevice()
                }
                else -> {
                    Log.d("BLE Controller", "Gatt connection state changed: $newState")
                    disconnectFromDevice()
                }
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            var serviceFound = false
            var characteristicFound = false

            gatt.services.forEach { service ->
                if (service.uuid.toString() == SERVICE_UUID) {
                    Log.d("BLE Controller", "\tService discovered: ${service.uuid}")
                    serviceFound = true
                    _serviceConnection.value = service

                    service.characteristics.forEach { characteristic ->
                        if (characteristic.uuid.toString() == CHAR_UUID) {
                        Log.d("BLE Controller", "\t\tCharacteristic discovered: ${characteristic.uuid}")
                            characteristicFound = true
                            _characteristicConnection.value = characteristic
                        }
                    }
                }
            }

            if (!serviceFound || !characteristicFound) disconnectFromDevice()
            else _connected.value = true
            _connecting.value = false
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            val pending = pendingTransaction ?: return

            if (status == BluetoothGatt.GATT_SUCCESS) {
                gatt.readCharacteristic(characteristic)
            }
            else {
                pending.completeExceptionally(Exception("Characteristic write failed"))
                pendingTransaction = null
            }
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            val pending = pendingTransaction ?: return

            if (status == BluetoothGatt.GATT_SUCCESS) {
                @Suppress("DEPRECATION")
                pending.complete(characteristic.value)
            }
            else {
                pending.completeExceptionally(Exception("Characteristic read failed"))
            }

            pendingTransaction = null
        }
    }

    @RequiresPermission(BLUETOOTH_SCAN)
    fun startScan() {
        bluetoothAdapter?.isEnabled?.let {
            if (!it) {
                Log.e("BLE Controller", "Bluetooth is not enabled")
                return
            }
        }

        if (bleScanner == null) {
            Log.e("BLE Controller", "Bluetooth scanner is not available")
            return
        }

        seenAddresses.clear()
        _foundDevices.value = emptyList()

        val filters = listOf<ScanFilter>()
        val settings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_BALANCED)
            .build()

        try {
            bleScanner?.startScan(filters, settings, scanCallback)
            Log.d("BLE Controller", "BLE Scan Started")
        }
        catch (e: Exception) {
            Log.e("BLE Controller", "BLE Scan Failed: ${e.message}")
        }
    }

    @RequiresPermission(BLUETOOTH_SCAN)
    fun stopScan() {
        bleScanner?.stopScan(scanCallback)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun connectToDevice(device: BluetoothDevice) {
        Log.d("BLE Controller", "Connecting to device: ${device.name}@${device.address}")
        _connecting.value = true
        device.connectGatt(appContext, false, gattCallback, TRANSPORT_LE)
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun disconnectFromDevice() {
        if (_gattConnection.value != null) {
            _gattConnection.value?.disconnect()
            _gattConnection.value?.close()
            _gattConnection.value = null
        }
        _serviceConnection.value = null
        _characteristicConnection.value = null

        _discovering.value = false
        _connecting.value = false
        _connected.value = false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    private suspend fun sendCommand(command: String) : String {
        if (_gattConnection.value == null ||
            _serviceConnection.value == null ||
            _characteristicConnection.value == null
        ) {
            Log.e("BLE Controller", "Gatt connection is not established")
            return "#No connection established"
        }

        Log.d("BLE Controller", "Sending command: $command")

        try {
            val result = _gattConnection.value!!.writeAndReadCharacteristic(
                _characteristicConnection.value!!,
                command.toByteArray()
            )

            val resultString = String(result!!)
            Log.d("BLE Controller", "Received response: $resultString")
            return resultString
        }
        catch (e: Exception) {
            return "#${e.message}"
        }
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun healthCheck() : Boolean {
        val response = sendCommand("${COMMANDS.HEALTH_CHECK}!")

        if (response[0] != '#') {
            failedHealthChecks = 0
            return true
        }

        failedHealthChecks++
        if (failedHealthChecks >= failedHealthChecksThreshold) {
            Log.e("BLE Controller", "Health check failed $failedHealthChecks times in a row")
            failedHealthChecks = 0
            disconnectFromDevice()
        }
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun audioHealthCheck() : Boolean {
        val response = sendCommand("${COMMANDS.AUDIO_HEALTH_CHECK}!")

        if (response[0] != '#') {
            failedAudioHealthChecks = 0
            return true
        }

        failedAudioHealthChecks++
        if (failedAudioHealthChecks >= failedHealthChecksThreshold) {
            Log.e("BLE Controller", "Audio health check failed $failedAudioHealthChecks times in a row")
            failedAudioHealthChecks = 0
        }
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun scanForAudioDevices(scanningTime: Long = 10000) : List<BTDevice>{
        if (_discovering.value) return emptyList()
        _discovering.value = true

        val startDiscoveryResponse = sendCommand("${COMMANDS.START_DISCOVERY}!")
        if (startDiscoveryResponse[0] == '#') {
            Log.e("BLE Controller", "Error starting audio device discovery: $startDiscoveryResponse")
            _discovering.value = false
            return emptyList()
        }

        delay(scanningTime)

        val stopDiscoveryResponse = sendCommand("${COMMANDS.STOP_DISCOVERY}!")
        if (stopDiscoveryResponse[0] == '#') {
            Log.e("BLE Controller", "Error stopping audio device discovery: $stopDiscoveryResponse")
            _discovering.value = false
            return emptyList()
        }

        delay(100)

        val getDevicesResponse = sendCommand("${COMMANDS.GET_DEVICES}!")
        if (getDevicesResponse[0] == '#') {
            Log.e("BLE Controller", "Error getting audio devices: $getDevicesResponse")
            return emptyList()
        }

        val result = mutableListOf<BTDevice>()

        val devicesString = getDevicesResponse.split('$')
        for (deviceString in devicesString) {
            val deviceInfo = deviceString.split('@')

            if (deviceInfo.size != 2) continue

            val deviceName = deviceInfo[0]
            val deviceAddress = deviceInfo[1]

            result.add(BTDevice(deviceName, deviceAddress))
        }

        _discovering.value = false
        failedHealthChecks = 0

        return result
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun pairAndConnectAudioDevice(device: BTDevice) : Boolean {
        if (_connecting.value) return false
        _connecting.value = true

        val pairResponse = sendCommand("${COMMANDS.PAIR_DEVICE}!${device.address}")
        if (pairResponse[0] == '#') {
            Log.e("BLE Controller", "Error pairing device: $pairResponse")
            _connecting.value = false
            return false
        }

        delay(10000)

        val connectResponse = sendCommand("${COMMANDS.CONNECT_DEVICE}!${device.address}")
        if (connectResponse[0] == '#') {
            Log.e("BLE Controller", "Error connecting to device: $connectResponse")
            _connecting.value = false
            return false
        }

        _connecting.value = false
        failedHealthChecks = 0

        return true
    }
}
