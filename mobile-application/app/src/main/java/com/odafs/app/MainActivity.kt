package com.odafs.app

import android.Manifest.permission.ACCESS_FINE_LOCATION
import android.Manifest.permission.BLUETOOTH
import android.Manifest.permission.BLUETOOTH_CONNECT
import android.Manifest.permission.BLUETOOTH_SCAN
import android.app.ComponentCaller
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS
import android.speech.RecognizerIntent
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.compose.ui.Modifier
import com.odafs.app.ble.BLEClient
import com.odafs.app.components.CommandButton
import com.odafs.app.ui.theme.ODAFSTheme
import java.util.Locale

class MainActivity : ComponentActivity() {
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val manager = application.getSystemService(BLUETOOTH_SERVICE) as BluetoothManager
        manager.adapter
    }

    val permissionLauncher = registerForActivityResult(
        contract = ActivityResultContracts.RequestMultiplePermissions()
    ) { it ->
        val allPermissionsGranted = it.values.all { it }

        if (!allPermissionsGranted) {
            val intent = Intent(ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                data = Uri.fromParts("package", packageName, null)
            }
            startActivity(intent)
        }
    }

    private val enableBluetoothIntentForResult = registerForActivityResult(
        contract = ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode != RESULT_OK) {
            showEnableBluetoothDialog()
        }
    }

    private fun showEnableBluetoothDialog() {
        bluetoothAdapter?.isEnabled?.let {
            if (!it) {
                val enableBluetoothIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                enableBluetoothIntentForResult.launch(enableBluetoothIntent)
            }
        }
    }

    @RequiresApi(Build.VERSION_CODES.S)
    override fun onStart() {
        super.onStart()
        permissionLauncher.launch(arrayOf(
            BLUETOOTH,
            BLUETOOTH_SCAN,
            BLUETOOTH_CONNECT,
            ACCESS_FINE_LOCATION
        ))
        showEnableBluetoothDialog()
        BLEClient.init(applicationContext)
    }

    @RequiresApi(Build.VERSION_CODES.TIRAMISU)
    @RequiresPermission(BLUETOOTH_CONNECT)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()
        setContent {
            ODAFSTheme {
                //NavigationController()
                CommandButton(modifier = Modifier)
            }
        }
    }
}
