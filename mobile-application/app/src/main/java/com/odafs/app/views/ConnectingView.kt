package com.odafs.app.views

import android.annotation.SuppressLint
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.odafs.app.ble.BLEController
import com.odafs.app.ble.BLEDevice
import com.odafs.app.ble.DEVICE_NAME
import com.odafs.app.ble.SERVICE_UUID
import com.odafs.app.components.TopBar
import kotlinx.coroutines.delay

@SuppressLint("MissingPermission")
@Composable
fun ConnectingView(navigateToScanning: () -> Unit) {
    var bleDevices by remember { mutableStateOf(emptyList<BLEDevice>()) }
    bleDevices = BLEController.foundDevices.collectAsState().value

    var connected by remember { mutableStateOf(false) }
    connected = BLEController.connected.collectAsState().value

    var connecting by remember { mutableStateOf(false) }
    connecting = BLEController.connecting.collectAsState().value

    LaunchedEffect(connected) {
        if (connected) navigateToScanning()

        while (true) {
            if (connecting) {
                delay(100)
                continue
            }

            BLEController.startScan()
            delay(5000)
            BLEController.stopScan()

            bleDevices@ for (bleDevice in bleDevices) {
                for (uuid in bleDevice.serviceUUIDs) {
                    if (bleDevice.device.name == DEVICE_NAME && uuid.toString() == SERVICE_UUID) {
                        BLEController.connectToDevice(bleDevice.device)
                        break@bleDevices
                    }
                }
            }
            delay(1000)
        }
    }

    Scaffold (topBar = { TopBar(title = "Conectando") }) { innerPadding ->
        Box(modifier = Modifier
            .fillMaxSize()
            .padding(horizontal = 20.dp)
        ) {
            Column (modifier = Modifier.align(Alignment.Center)) {
                CircularProgressIndicator(
                    color = MaterialTheme.colorScheme.primary,
                    strokeWidth = 6.dp,
                    modifier = Modifier
                        .size(100.dp)
                        .align(Alignment.CenterHorizontally)
                )

                Spacer(modifier = Modifier.height(10.dp))

                Text(
                    text = "Buscando Dispositivo",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold,
                    textAlign = TextAlign.Center,
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(modifier = Modifier.height(5.dp))

                Text(
                    text = "Asegúrate de que el dispositivo está encendido",
                    style = MaterialTheme.typography.bodyLarge,
                    textAlign = TextAlign.Center,
                    modifier = Modifier.fillMaxWidth()
                )

//                LazyColumn {
//                    items(bleDevices) { device ->
//                        Text(text = "${device.device.name}@${device.device.address}")
//                    }
//                }
            }
        }
    }
}

@Preview
@Composable
fun ConnectingViewPreview() {
    ConnectingView {}
}
