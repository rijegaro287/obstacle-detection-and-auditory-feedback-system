package com.odafs.app.ble

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue

object BLEDeviceManager {
    var foundDevices = mutableStateListOf<BTAudioDevice>()
    var selectedDevice by mutableStateOf<BTAudioDevice?>(null)
    fun selectDevice(indexSpoken: Int): String {
        val deviceIndex = indexSpoken - 1
        if (deviceIndex in foundDevices.indices) {
            selectedDevice = foundDevices[deviceIndex]
            return selectedDevice!!.name
        }
        return ""
    }
}
