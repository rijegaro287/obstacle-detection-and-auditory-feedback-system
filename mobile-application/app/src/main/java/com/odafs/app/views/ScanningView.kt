package com.odafs.app.views

import android.Manifest
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattService
import android.os.Build
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.compose.foundation.background
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
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Mic
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.odafs.app.ble.BLEController
import com.odafs.app.components.CommandButton
import com.odafs.app.components.DeviceCard
import com.odafs.app.components.TopBar
import androidx.compose.runtime.collectAsState

data class Device(val name: String, val address: String)

val deviceList = listOf(
    Device(name = "device 1", address = "address 1"),
    Device(name = "device 2", address = "address 2"),
    Device(name = "device 3", address = "address 3"),
    Device(name = "device 4", address = "address 4")
)

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@Composable
fun ScanningView(
    navigateToConnecting: () -> Unit,
    navigateToControls: (String) -> Unit
) {
    var connected by remember { mutableStateOf(false) }
    connected = BLEController.connected.collectAsState().value

    LaunchedEffect(connected) {
        if (!connected) navigateToConnecting()
    }

    LaunchedEffect(Unit) {
        BLEController.sendCommand("Hello!!!")
    }

    Scaffold(
        topBar = { TopBar(title = "Conéctate a un dispositivo") }
    ) { innerPadding ->
        Box(modifier = Modifier
            .fillMaxSize()
            .padding(
                top = innerPadding.calculateTopPadding(),
                bottom = 75.dp,
                start = 18.dp,
                end = 18.dp
            )
        ) {
            Column(modifier = Modifier.align(Alignment.TopCenter)) {
                Box(modifier = Modifier.fillMaxWidth()) {
                    Text(
                        text = "Dispositivos disponibles:",
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.align(Alignment.CenterStart)
                    )

                    IconButton(
                        onClick = {},
                        modifier = Modifier
                            .align(Alignment.CenterEnd)
                            .background(
                                color = MaterialTheme.colorScheme.surfaceContainer,
                                shape = CircleShape
                            )
                            .size(40.dp)
                    ) {
                        Icon(
                            imageVector = Icons.Default.Refresh,
                            contentDescription = "Botón para refrescar la lista de dispositivos de audio disponibles",
                            modifier = Modifier.size(35.dp)
                        )
                    }
                }

                Spacer(modifier = Modifier.height(20.dp))

                LazyColumn {
                    items(deviceList) { device ->
                        DeviceCard(device.name) {
                            navigateToControls(device.name)
                        }
                        Spacer(modifier = Modifier.height(8.dp))
                    }
                }
            }

            CommandButton(modifier = Modifier.align(Alignment.BottomCenter))
        }
    }
}

@Preview
@Composable
fun ScanningViewPreview () {
    ScanningView({}, {})
}
