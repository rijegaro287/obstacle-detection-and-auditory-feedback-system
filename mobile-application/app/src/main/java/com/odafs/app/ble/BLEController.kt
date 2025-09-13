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
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.bluetooth.le.ScanSettings.SCAN_MODE_LOW_LATENCY
import android.content.Context
import android.os.Build
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

const val DEVICE_NAME = "odafs"
const val SERVICE_UUID = "9b19df40-4042-4479-0000-131cd24590be"
const val CHAR_UUID = "9b19df40-4042-4479-0001-131cd24590be"

data class BLEDevice(
    val device: BluetoothDevice,
    val serviceUUIDs: List<ParcelUuid>
)

object BLEController {
    private var appContext: Application? = null

    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bleScanner: BluetoothLeScanner? = null

    private val seenAddresses = mutableSetOf<String>()
    private val _foundDevices = MutableStateFlow<List<BLEDevice>>(emptyList())
    val foundDevices: StateFlow<List<BLEDevice>> = _foundDevices.asStateFlow()

    private val _gattConnection = MutableStateFlow<BluetoothGatt?>(null)
    val gattConnection: StateFlow<BluetoothGatt?> = _gattConnection.asStateFlow()

    private val _serviceConnection = MutableStateFlow<BluetoothGattService?>(null)
    val serviceConnection: StateFlow<BluetoothGattService?> = _serviceConnection.asStateFlow()

    private val _characteristicConnection = MutableStateFlow<BluetoothGattCharacteristic?>(null)
    val characteristicConnection: StateFlow<BluetoothGattCharacteristic?> = _characteristicConnection.asStateFlow()

    private val _connecting = MutableStateFlow(false)
    val connecting: StateFlow<Boolean> = _connecting.asStateFlow()

    private val _connected = MutableStateFlow(false)
    val connected: StateFlow<Boolean> = _connected.asStateFlow()

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

    private val gattCallback = object : BluetoothGattCallback() {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            Log.d("BLE Controller", "Gatt connection state changed: $newState")
            when (newState) {
                BluetoothGatt.STATE_CONNECTED -> {
                    _gattConnection.value = gatt
                    gatt.discoverServices()
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    _connecting.value = false
                    _gattConnection.value = null
                }
                else -> {
                    Log.d("BLE Controller", "Gatt connection state changed: $newState")
                    _connecting.value = false
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
            .setScanMode(SCAN_MODE_LOW_LATENCY)
            .setReportDelay(0L)
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
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun sendCommand(command: String) {
        if (_gattConnection.value == null ||
            _serviceConnection.value == null ||
            _characteristicConnection.value == null
        ) {
            Log.e("BLE Controller", "Gatt connection is not established")
            return
        }

        Log.d("BLE Controller", "Sending command: $command")
        val commandBytes = command.toByteArray()
        _characteristicConnection.value?.let {
            _gattConnection.value?.writeCharacteristic(
                it,
                commandBytes,
                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            )
        }
    }
}
