package com.odafs.app.ble

import android.Manifest
import android.os.Build
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import kotlinx.coroutines.delay

object BLEController {


//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun audioHealthCheck() : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.AUDIO_HEALTH_CHECK}!")
//
//        if (response[0] != '#') {
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Audio health check failed: $response")
//        failedAudioHealthChecks++
//        if (failedAudioHealthChecks >= failedHealthChecksThreshold) {
//            Log.e("BLE Controller", "Audio health check failed $failedAudioHealthChecks times in a row")
//            failedAudioHealthChecks = 0
//            _connectedAudioDevice.value = null
//        }
//        return false
//    }

//

//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun disconnectAudioDevice() : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.DISCONNECT_DEVICE}!")
//
//        if (response[0] != '#') {
//            _connectedAudioDevice.value = null
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Error disconnecting from device: $response")
//        return false
//    }
//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun startAudioFeedback() : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.START_FEEDBACK}!")
//
//        if (response[0] != '#') {
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Error starting feedback: $response")
//        return false
//    }
//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun stopAudioFeedback() : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.STOP_FEEDBACK}!")
//
//        if (response[0] != '#') {
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Error stopping feedback: $response")
//        return false
//    }
//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun setAudioVolume(volume: Int) : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.SET_VOLUME}!${volume}")
//
//        if (response[0] != '#') {
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Error setting volume: $response")
//        return false
//    }
//
//    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
//    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//    suspend fun setFeedbackMode(mode: String) : Boolean {
//        val response = BLEClient.GATTConnection.sendCommand("${Commands.SET_FEEDBACK_MODE}!${mode}")
//
//        if (response[0] != '#') {
//            failedAudioHealthChecks = 0
//            return true
//        }
//
//        Log.e("BLE Controller", "Error setting feedback mode: $response")
//        return false
//    }
}
