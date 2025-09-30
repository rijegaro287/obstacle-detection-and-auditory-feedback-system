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
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

const val DEVICE_NAME = "odafs"
const val SERVICE_UUID = "9b19df40-4042-4479-0000-131cd24590be"
const val CHAR_UUID = "9b19df40-4042-4479-0001-131cd24590be"

object Delays {
    const val MISC_DELAY = 100L
    const val SERVICE_SCAN_DELAY = 5000L
    const val AUDIO_DEVICE_SCAN_DELAY = 8000L
    const val HEALTH_CHECK_DELAY = 2050L
    const val AUDIO_HEALTH_CHECK_DELAY = 2350L
}

data class BLEDevice(
    val device: BluetoothDevice,
    val serviceUUIDs: List<ParcelUuid>
)

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
                    // _gattConnection.value = gatt
                    gatt.discoverServices()
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Disconnected")
                    // disconnectFromDevice()
                }
                else -> {
                    Log.d("BLE Controller", "Gatt connection state changed: $newState")
                    // disconnectFromDevice()
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
                // disconnectFromDevice()
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
        // private val _gattConnection = MutableStateFlow<BluetoothGatt?>(null)
        internal val _characteristicConnection = MutableStateFlow<BluetoothGattCharacteristic?>(null)

        internal var _pendingTransaction: CompletableDeferred<ByteArray?>? = null
    }
}