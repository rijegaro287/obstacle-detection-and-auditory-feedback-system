package com.odafs.app.ble

import android.Manifest
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.util.Log
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.flow.MutableStateFlow



object GATTConnection {
//    private val _gattConnection = MutableStateFlow<BluetoothGatt?>(null)
//    private val _serviceConnection = MutableStateFlow<BluetoothGattService?>(null)
    private val _characteristicConnection = MutableStateFlow<BluetoothGattCharacteristic?>(null)

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    fun onBLEConnected(gatt: BluetoothGatt) {
        // _gattConnection.value = gatt
        gatt.discoverServices()
    }

    fun onBLEDisconnected() {
        // disconnectFromDevice()
    }

    fun onServicesDiscovered(gatt: BluetoothGatt,
                             connected_state: MutableStateFlow<Boolean>,
                             connecting_state: MutableStateFlow<Boolean>
    ) {
        var serviceFound = false
        var characteristicFound = false

        gatt.services.forEach { service ->
            if (service.uuid.toString() == SERVICE_UUID) {
                Log.d("BLE Controller", "\tService discovered: ${service.uuid}")
                serviceFound = true
                // _serviceConnection.value = service

                service.characteristics.forEach { characteristic ->
                    if (characteristic.uuid.toString() == CHAR_UUID) {
                        Log.d("BLE Controller", "\t\tCharacteristic discovered: ${characteristic.uuid}")
                        characteristicFound = true
                        _characteristicConnection.value = characteristic
                    }
                }
            }
        }

         if (!serviceFound || !characteristicFound) {
             // disconnectFromDevice()
         }
         else {
             connected_state.value = true
         }
        connecting_state.value = false
    }
}