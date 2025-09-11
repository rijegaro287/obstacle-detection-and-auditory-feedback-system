package com.odafs.app.ble

import android.Manifest.permission.BLUETOOTH_SCAN
import android.annotation.SuppressLint
import android.app.Application
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.bluetooth.le.ScanSettings.SCAN_MODE_LOW_LATENCY
import android.content.ContentValues.TAG
import android.content.Context
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresPermission
import androidx.lifecycle.AndroidViewModel
import com.google.android.gms.common.zzq
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

data class BLEDevice(
    val name: String,
    val address: String,
    val serviceUUIDs: List<ParcelUuid>?
)

class BLEController(application: Application) : AndroidViewModel(application) {
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val manager = application.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        manager.adapter
    }

    private val bleScanner: BluetoothLeScanner? by lazy {
        bluetoothAdapter?.bluetoothLeScanner
    }

    private val _foundDevices = MutableStateFlow<List<BLEDevice>>(emptyList())
    val foundDevices: StateFlow<List<BLEDevice>> = _foundDevices.asStateFlow()
    private val seenAddresses = mutableSetOf<String>()

    @RequiresPermission(BLUETOOTH_SCAN)
    fun startScan() {
        Log.d(TAG, "BLE Start Called")

        bluetoothAdapter?.isEnabled?.let {
            if (!it) {
                Log.e(TAG, "Bluetooth is not enabled")
                return
            }
        }

        if (bleScanner == null) {
            Log.e(TAG, "Bluetooth scanner is not available")
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
            Log.d(TAG, "BLE Scan Started")
        }
        catch (e: Exception) {
            Log.e(TAG, "BLE Scan Failed: ${e.message}")
        }
    }

    @RequiresPermission(BLUETOOTH_SCAN)
    fun stopScan() {
        bleScanner?.stopScan(scanCallback)
    }

    private val scanCallback = object : ScanCallback() {
        @SuppressLint("MissingPermission")
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            result.let {
                val bleDevice = BLEDevice(
                    name = it?.device?.name ?: "Unknown",
                    address = it?.device?.address ?: "",
                    serviceUUIDs = it?.scanRecord?.serviceUuids?.toList()
                )

                Log.d(TAG,"===========================================================")
                Log.d(TAG,"BLE Device Found: ${bleDevice.name}@${bleDevice.address}")

                if (bleDevice.serviceUUIDs != null) {
                    for (uuid in bleDevice.serviceUUIDs) {
                        Log.d(TAG,uuid.toString())
                    }
                }

                if (seenAddresses.add(bleDevice.address)) {
                    _foundDevices.value = _foundDevices.value + bleDevice
                }
            }
        }

        @SuppressLint("MissingPermission")
        override fun onBatchScanResults(results: MutableList<ScanResult>?) {
            Log.d(TAG, "BLE Batch Scan Results: $results")
            results?.forEach { device ->
                device.let {
                    val bleDevice = BLEDevice(
                        name = it.device?.name ?: "Unknown",
                        address = it.device?.address ?: "",
                        serviceUUIDs = it.device?.uuids?.toList()
                    )

                    if (bleDevice.serviceUUIDs != null) {
                        for (uuid in bleDevice.serviceUUIDs) {
                            Log.d(TAG,uuid.toString())
                        }
                    }

                    if (seenAddresses.add(bleDevice.address)) {
                        _foundDevices.value = _foundDevices.value + bleDevice
                    }
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("BLE", "Scan failed: $errorCode")
        }
    }
}
