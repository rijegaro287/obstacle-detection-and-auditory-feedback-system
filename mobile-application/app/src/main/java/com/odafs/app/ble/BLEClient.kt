package com.odafs.app.ble

import android.Manifest
import android.Manifest.permission.BLUETOOTH_SCAN
import android.app.Application
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothDevice.TRANSPORT_LE
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
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

data class BLEDevice(
    val info: BluetoothDevice,
    val serviceUUIDs: List<ParcelUuid>
)

data class BTAudioDevice(
    val name: String,
    val address: String
)

object Delays {
    const val READ_WRITE_CHAR_TIMEOUT = 10000L
    const val ERROR_TIMEOUT = 5000L
    const val MISC_DELAY = 100L
    const val SERVICE_SCAN_DELAY = 5000L
    const val AUDIO_DEVICE_SCAN_DELAY = 8000L
    const val HEALTH_CHECK_DELAY = 2050L
    const val AUDIO_HEALTH_CHECK_DELAY = 2350L

    const val PAIR_AND_CONNECT_DELAY = 500L
}

object Commands {
    const val HEALTH_CHECK = "health_check"
    const val AUDIO_HEALTH_CHECK = "audio_health_check"
    const val START_DISCOVERY = "start_discovery"
    const val STOP_DISCOVERY = "stop_discovery"
    const val GET_DEVICES = "get_devices"
    const val PAIR_DEVICE = "pair_device"
    const val CONNECT_DEVICE = "connect_device"
    const val DISCONNECT_DEVICE = "disconnect_device"
    const val START_FEEDBACK = "start_feedback"
    const val STOP_FEEDBACK = "stop_feedback"
    const val SET_VOLUME = "set_volume"
    const val SET_FEEDBACK_MODE = "set_feedback_mode"
}

object FeedbackModes {
    const val NON_VERBAL_FEEDBACK = "non_verbal"
    const val VERBAL_FEEDBACK = "verbal"

    const val NON_VERBAL_FEEDBACK_CODE = 0
    const val VERBAL_FEEDBACK_CODE = 1

    fun map_feedback_mode(mode: String) : Int {
        if (mode == NON_VERBAL_FEEDBACK) {
            return NON_VERBAL_FEEDBACK_CODE
        }
        else if (mode == VERBAL_FEEDBACK) {
            return VERBAL_FEEDBACK_CODE
        }
        else {
            return -1
        }
    }
}

object BLEClient {
    private var appContext: Application? = null
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bleScanner: BluetoothLeScanner? = null

    fun init(context: Context) {
        appContext = context.applicationContext as Application

        val manager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = manager.adapter

        bleScanner = bluetoothAdapter?.bluetoothLeScanner
    }

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun disconnect() {
        if (GATTConnection._gattConnection.value != null) {
            GATTConnection._gattConnection.value?.disconnect()
            GATTConnection._gattConnection.value?.close()
            GATTConnection._gattConnection.value = null
        }
        GATTConnection._characteristicConnection.value = null

        DeviceConnection._connected.value = false
        DeviceConnection._connecting.value = false

        GATTConnection._discovering.value = false
        GATTConnection._connecting.value = false
        GATTConnection._connectedAudioDevice.value = null
    }

    private val deviceScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            result.let {
                val device = it?.device
                val serviceUUIDs = it?.scanRecord?.serviceUuids

                if (device != null && serviceUUIDs != null) {
                    if (DeviceConnection._seenAddresses.add(device.address)) {
                        val bleDevice = BLEDevice(device, serviceUUIDs)
                        DeviceConnection._foundDevices.value = DeviceConnection._foundDevices.value + bleDevice
                    }
                }
            }
        }

        override fun onBatchScanResults(results: MutableList<ScanResult>?) {
            Log.d("BLE Controller", "BLE Batch Scan Results: $results")
            results?.forEach { device ->
                device.let {
                    val device = it.device
                    val serviceUUIDs = it.scanRecord?.serviceUuids

                    if (device != null && serviceUUIDs != null) {
                        if (DeviceConnection._seenAddresses.add(device.address)) {
                            val bleDevice = BLEDevice(device, serviceUUIDs)
                            DeviceConnection._foundDevices.value = DeviceConnection._foundDevices.value + bleDevice
                        }
                    }
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("BLE", "Scan failed: $errorCode")
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothGatt.STATE_CONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Connected")
                    GATTConnection._gattConnection.value = gatt
                    gatt.discoverServices()
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Disconnected")
                    disconnect()
                }
                else -> {
                    Log.d("BLE Controller", "Gatt connection state changed: $newState")
                    disconnect()
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
                    service.characteristics.forEach { characteristic ->
                        if (characteristic.uuid.toString() == CHAR_UUID) {
                            Log.d("BLE Controller", "\t\tCharacteristic discovered: ${characteristic.uuid}")
                            characteristicFound = true
                            GATTConnection._characteristicConnection.value = characteristic
                        }
                    }
                }
            }

            if (!serviceFound || !characteristicFound) {
                disconnect()
            }
            else {
                DeviceConnection._connected.value = true
            }
            DeviceConnection._connecting.value = false
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            val pending = GATTConnection._pendingTransaction ?: return

            if (status == BluetoothGatt.GATT_SUCCESS) {
                gatt.readCharacteristic(characteristic)
            }
            else {
                pending.completeExceptionally(Exception("Characteristic write failed"))
                GATTConnection._pendingTransaction = null
            }
        }

        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            val pending = GATTConnection._pendingTransaction ?: return

            if (status == BluetoothGatt.GATT_SUCCESS) {
                @Suppress("DEPRECATION")
                pending.complete(characteristic.value)
            }
            else {
                pending.completeExceptionally(Exception("Characteristic read failed"))
            }

            GATTConnection._pendingTransaction = null
        }
    }

    object DeviceConnection {
        internal val _seenAddresses = mutableSetOf<String>()
        internal val _foundDevices = MutableStateFlow<List<BLEDevice>>(emptyList())
        val foundDevices: StateFlow<List<BLEDevice>> = _foundDevices.asStateFlow()

        internal val _connected = MutableStateFlow(false)
        val connected: StateFlow<Boolean> = _connected.asStateFlow()

        internal val _connecting = MutableStateFlow(false)
        val connecting: StateFlow<Boolean> = _connecting.asStateFlow()

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

            _seenAddresses.clear()
            _foundDevices.value = emptyList()

            val filters = listOf<ScanFilter>()
            val settings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_BALANCED)
                .build()

            try {
                bleScanner?.startScan(filters, settings, deviceScanCallback)
                Log.d("BLE Controller", "BLE Scan Started")
            }
            catch (e: Exception) {
                Log.e("BLE Controller", "BLE Scan Failed: ${e.message}")
            }
        }

        @RequiresPermission(BLUETOOTH_SCAN)
        fun stopScan() {
            bleScanner?.stopScan(deviceScanCallback)
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        fun connectToDevice(device: BluetoothDevice) {
            Log.d("BLE Controller", "Connecting to device: ${device.name}@${device.address}")
            _connecting.value = true
            device.connectGatt(appContext, false, gattCallback, TRANSPORT_LE)
        }
    }

    object GATTConnection {
        internal val _gattConnection = MutableStateFlow<BluetoothGatt?>(null)
        internal val _characteristicConnection = MutableStateFlow<BluetoothGattCharacteristic?>(null)

        internal val _connectedAudioDevice = MutableStateFlow<BTAudioDevice?>(null)
        val connectedAudioDevice: StateFlow<BTAudioDevice?> = _connectedAudioDevice.asStateFlow()

        internal var _foundAudioDevices = MutableStateFlow<List<BTAudioDevice>>(emptyList())
        val foundAudioDevices: StateFlow<List<BTAudioDevice>> = _foundAudioDevices.asStateFlow()

        internal val _discovering = MutableStateFlow(false)
        val discovering: StateFlow<Boolean> = _discovering.asStateFlow()

        internal val _connecting = MutableStateFlow(false)
        val connecting: StateFlow<Boolean> = _connecting.asStateFlow()

        internal var _pendingTransaction: CompletableDeferred<ByteArray?>? = null

        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        suspend fun BluetoothGatt.writeAndReadCharacteristic(
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) : ByteArray? {
            if (_pendingTransaction != null) {
                Log.e("BLE Controller", "Another transaction is pending")
                return null
            }

            val pending = CompletableDeferred<ByteArray?>()
            _pendingTransaction = pending

            val startedTransaction = writeCharacteristic(
                characteristic,
                value,
                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            )

            if (startedTransaction != BluetoothStatusCodes.SUCCESS) {
                pending.completeExceptionally(Exception("Characteristic write failed"))
                _pendingTransaction = null
                return null
            }

            try {
                return withTimeout(Delays.READ_WRITE_CHAR_TIMEOUT) {
                    _pendingTransaction?.await()
                }
            }
            catch (e: CancellationException) {
                _pendingTransaction?.cancel()
                _pendingTransaction = null
                throw e
            }
            catch (e: Throwable) {
                _pendingTransaction?.completeExceptionally(e)
                _pendingTransaction = null
                throw e
            }
            finally {
                _pendingTransaction = null
            }
        }

        @RequiresApi(Build.VERSION_CODES.TIRAMISU)
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        suspend fun sendCommand(command: String) : String {
            if (_gattConnection.value == null ||
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

                if (result == null) {
                    return "#Error sending command $command"
                }

                val resultString = String(result)
                Log.d("BLE Controller", "Received response: $resultString")
                return resultString
            }
            catch (e: Exception) {
                return "#${e.message}"
            }
        }
    }

    object Controls {
        internal val _playingFeedback = MutableStateFlow<Boolean>(false)
        val playingFeedback: StateFlow<Boolean> = _playingFeedback.asStateFlow()

        internal val _volume = MutableStateFlow(0.5f)
        val volume: StateFlow<Float> = _volume.asStateFlow()

        internal val _feedbackMode = MutableStateFlow(FeedbackModes.NON_VERBAL_FEEDBACK)
        val feedbackMode: StateFlow<String> = _feedbackMode.asStateFlow()
    }

    private val failedHealthChecksThreshold = 3
    private var failedHealthChecks = 0
    private var failedAudioHealthChecks = 0

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun healthCheck() : Boolean {
        val response = GATTConnection.sendCommand("${Commands.HEALTH_CHECK}!")

        if (response[0] != '#') {
            failedHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Health check failed: $response")
        failedHealthChecks++
        if (failedHealthChecks >= failedHealthChecksThreshold) {
            Log.e("BLE Controller", "Health check failed $failedHealthChecks times in a row")
            failedHealthChecks = 0
            disconnect()
        }
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun audioHealthCheck() : Boolean {
        val response = GATTConnection.sendCommand("${Commands.AUDIO_HEALTH_CHECK}!")

        if (response[0] != '#') {
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Audio health check failed: $response")
        failedAudioHealthChecks++
        if (failedAudioHealthChecks >= failedHealthChecksThreshold) {
            Log.e("BLE Controller", "Audio health check failed $failedAudioHealthChecks times in a row")
            failedAudioHealthChecks = 0
            GATTConnection._connectedAudioDevice.value = null
        }
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun scanForAudioDevices(scanningTime: Long = 10000): Boolean {
        if (GATTConnection._discovering.value) return false
        GATTConnection._discovering.value = true

        val startDiscoveryResponse = GATTConnection.sendCommand("${Commands.START_DISCOVERY}!")
        if (startDiscoveryResponse[0] == '#') {
            Log.e("BLE Controller", "Error starting audio device discovery: $startDiscoveryResponse")
            GATTConnection._discovering.value = false
            return false
        }

        delay(scanningTime)

        val stopDiscoveryResponse = GATTConnection.sendCommand("${Commands.STOP_DISCOVERY}!")
        if (stopDiscoveryResponse[0] == '#') {
            Log.e("BLE Controller", "Error stopping audio device discovery: $stopDiscoveryResponse")
            GATTConnection._discovering.value = false
            return false
        }

        delay(Delays.MISC_DELAY)

        val getDevicesResponse = GATTConnection.sendCommand("${Commands.GET_DEVICES}!")
        if (getDevicesResponse[0] == '#') {
            Log.e("BLE Controller", "Error getting audio devices: $getDevicesResponse")
            return false
        }

        val result = mutableListOf<BTAudioDevice>()

        val devicesString = getDevicesResponse.split('$')
        for (deviceString in devicesString) {
            val deviceInfo = deviceString.split('@')

            if (deviceInfo.size != 2) continue

            val deviceName = deviceInfo[0]
            val deviceAddress = deviceInfo[1]

            result.add(BTAudioDevice(deviceName, deviceAddress))
        }

        GATTConnection._foundAudioDevices.value = result

        GATTConnection._discovering.value = false
        failedHealthChecks = 0
        return true
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun pairAndConnectAudioDevice(device: BTAudioDevice) : Boolean {
        if (GATTConnection._connecting.value) return false
        GATTConnection._connecting.value = true

        val pairResponse = GATTConnection.sendCommand("${Commands.PAIR_DEVICE}!${device.address}")
        if (pairResponse[0] == '#') {
            Log.e("BLE Controller", "Error pairing device: $pairResponse")
            GATTConnection._connectedAudioDevice.value = null
            GATTConnection._connecting.value = false
            return false
        }

        delay(Delays.PAIR_AND_CONNECT_DELAY)

        val connectResponse = GATTConnection.sendCommand("${Commands.CONNECT_DEVICE}!${device.address}")
        if (connectResponse[0] == '#') {
            Log.e("BLE Controller", "Error connecting to device: $connectResponse")
            GATTConnection._connectedAudioDevice.value = null
            GATTConnection._connecting.value = false
            return false
        }

        GATTConnection._connectedAudioDevice.value = device
        GATTConnection._connecting.value = false
        failedHealthChecks = 0

        return true
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun startAudioFeedback() : Boolean {
        val response = GATTConnection.sendCommand("${Commands.START_FEEDBACK}!")

        if (response[0] != '#') {
            Controls._playingFeedback.value = true
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Error starting feedback: $response")
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun stopAudioFeedback() : Boolean {
        val response = GATTConnection.sendCommand("${Commands.STOP_FEEDBACK}!")

        if (response[0] != '#') {
            Controls._playingFeedback.value = false
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Error stopping feedback: $response")
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun setAudioVolume(volume: Float) : Boolean {
        val volumeInt = (100 * volume).toInt()
        val response = GATTConnection.sendCommand("${Commands.SET_VOLUME}!${volumeInt}")

        if (response[0] != '#') {
            Controls._volume.value = volume
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Error setting volume: $response")
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun setFeedbackMode(mode: String) : Boolean {
        val response = GATTConnection.sendCommand("${Commands.SET_FEEDBACK_MODE}!${mode}")

        if (response[0] != '#') {
            Controls._feedbackMode.value = mode
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Error setting feedback mode: $response")
        return false
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    suspend fun disconnectAudioDevice() : Boolean {
        val response = GATTConnection.sendCommand("${Commands.DISCONNECT_DEVICE}!")

        if (response[0] != '#') {
            GATTConnection._connectedAudioDevice.value = null
            failedAudioHealthChecks = 0
            return true
        }

        Log.e("BLE Controller", "Error disconnecting from device: $response")
        return false
    }
}
