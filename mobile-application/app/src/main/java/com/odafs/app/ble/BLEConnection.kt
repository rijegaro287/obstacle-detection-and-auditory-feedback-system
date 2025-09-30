package com.odafs.app.ble

import android.Manifest
import android.Manifest.permission.BLUETOOTH_SCAN
import android.app.Application
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothDevice.TRANSPORT_LE
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
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
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlin.collections.forEach
import kotlin.collections.plus

object BLEConnection {
    private var appContext: Application? = null
    private var bluetoothAdapter: BluetoothAdapter? = null
    private var bleScanner: BluetoothLeScanner? = null

    private val seenAddresses = mutableSetOf<String>()
    private val _foundDevices = MutableStateFlow<List<BLEDevice>>(emptyList())
    val foundDevices: StateFlow<List<BLEDevice>> = _foundDevices.asStateFlow()

    private val _connected = MutableStateFlow(false)
    val connected: StateFlow<Boolean> = _connected.asStateFlow()

    private val _connecting = MutableStateFlow(false)
    val connecting: StateFlow<Boolean> = _connecting.asStateFlow()

    fun init(context: Context) {
        appContext = context.applicationContext as Application

        val manager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = manager.adapter

        bleScanner = bluetoothAdapter?.bluetoothLeScanner
    }

    private val scanCallback = object : ScanCallback() {
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
            when (newState) {
                BluetoothGatt.STATE_CONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Connected")
                    GATTConnection.onBLEConnected(gatt)
                }
                BluetoothGatt.STATE_DISCONNECTED -> {
                    Log.d("BLE Controller", "Gatt connection state changed: Disconnected")
                    GATTConnection.onBLEDisconnected()
                }
                else -> {
                    Log.d("BLE Controller", "Gatt connection state changed: $newState")
                    GATTConnection.onBLEDisconnected()
                }
            }
        }

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            GATTConnection.onServicesDiscovered(gatt, _connected, _connecting)
        }
//
//        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//        override fun onCharacteristicWrite(
//            gatt: BluetoothGatt,
//            characteristic: BluetoothGattCharacteristic,
//            status: Int
//        ) {
//            val pending = pendingTransaction ?: return
//
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                gatt.readCharacteristic(characteristic)
//            }
//            else {
//                pending.completeExceptionally(Exception("Characteristic write failed"))
//                pendingTransaction = null
//            }
//        }
//
//        override fun onCharacteristicRead(
//            gatt: BluetoothGatt,
//            characteristic: BluetoothGattCharacteristic,
//            status: Int
//        ) {
//            val pending = pendingTransaction ?: return
//
//            if (status == BluetoothGatt.GATT_SUCCESS) {
//                @Suppress("DEPRECATION")
//                pending.complete(characteristic.value)
//            }
//            else {
//                pending.completeExceptionally(Exception("Characteristic read failed"))
//            }
//
//            pendingTransaction = null
//        }
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
}
