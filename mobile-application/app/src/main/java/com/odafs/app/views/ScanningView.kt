package com.odafs.app.views

import android.Manifest
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
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.CircularProgressIndicator
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
import com.odafs.app.components.CommandButton
import com.odafs.app.components.DeviceCard
import com.odafs.app.components.TopBar
import androidx.compose.runtime.collectAsState
import androidx.compose.ui.graphics.BlurEffect
import androidx.compose.ui.text.style.TextAlign
import com.odafs.app.ble.BLEClient
import com.odafs.app.ble.BTAudioDevice
import com.odafs.app.ble.Delays
import com.odafs.app.components.AutoDismissDialog
import kotlinx.coroutines.delay

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@Composable
fun ScanningView(
    navigateToConnecting: () -> Unit,
    navigateToControls: (String) -> Unit
) {
    var connectedAudioDevice by remember { mutableStateOf<BTAudioDevice?>(null) }
    connectedAudioDevice = BLEClient.GATTConnection.connectedAudioDevice.collectAsState().value

    var foundAudioDevices by remember { mutableStateOf(emptyList<BTAudioDevice>()) }
    foundAudioDevices = BLEClient.GATTConnection.foundAudioDevices.collectAsState().value

    var connected by remember { mutableStateOf(false) }
    connected = BLEClient.DeviceConnection.connected.collectAsState().value

    var discovering by remember { mutableStateOf(false) }
    discovering = BLEClient.GATTConnection.discovering.collectAsState().value

    var connecting by remember { mutableStateOf(false) }
    connecting = BLEClient.GATTConnection.connecting.collectAsState().value

    var reloadClicked by remember { mutableStateOf(false) }
    var selectedDevice by remember { mutableStateOf<BTAudioDevice?>(null) }

    var showError by remember { mutableStateOf(false) }
    var errorTitle by remember { mutableStateOf("") }
    var errorMessage by remember { mutableStateOf("") }
    var onDismiss : () -> Unit by remember { mutableStateOf({}) }

    LaunchedEffect(connected) {
        if (!connected) {
            errorTitle = "Conexión perdida"
            errorMessage = "Se perdió la conexión con el dispositivo de procesamiento"
            showError = true
            onDismiss = {
                showError = false
                navigateToConnecting()
            }
        }
    }

    LaunchedEffect(Unit) {
        while (true) {
            delay(Delays.HEALTH_CHECK_DELAY)
            BLEClient.healthCheck()
        }
    }

    LaunchedEffect(Unit) {
        if (!connecting && !discovering) {
            Log.d("BLE Controller", "Scanning for audio devices")
            BLEClient.scanForAudioDevices(Delays.AUDIO_DEVICE_SCAN_DELAY)
        }
    }

    LaunchedEffect(reloadClicked) {
        if (!discovering && !connecting && reloadClicked) {
            Log.d("BLE Controller", "Reloading devices")
            BLEClient.scanForAudioDevices(Delays.AUDIO_DEVICE_SCAN_DELAY)
        }
        reloadClicked = false
    }

    LaunchedEffect(selectedDevice) {
        if (!connecting && selectedDevice != null) {
            Log.d("BLE Controller", "Connecting to ${selectedDevice!!.name}")
            val connectionEstablished = BLEClient.pairAndConnectAudioDevice(selectedDevice!!)
            if (!connectionEstablished) {
                errorTitle = "Conexión fallida"
                errorMessage = "No se pudo establecer la conexión con el dispositivo de audio"
                showError = true
                onDismiss = { showError = false }
                selectedDevice = null
            }
        }
        selectedDevice = null
    }

    LaunchedEffect(connectedAudioDevice) {
        if (connectedAudioDevice != null) {
            navigateToControls(connectedAudioDevice!!.name)
        }
    }

    // función interna que selecciona un dispositivo por número
    fun selectDeviceByNumber(indexSpoken: Int): String {
        val deviceIndex = indexSpoken - 1
        if (BLEDeviceManager.foundDevices.isNotEmpty() && deviceIndex in BLEDeviceManager.foundDevices.indices) {
            val device = BLEDeviceManager.foundDevices[deviceIndex]
            BLEDeviceManager.selectedDevice = device
            return device.name
        }
        return ""
    }


    AutoDismissDialog(
        visible = showError,
        title = errorTitle,
        message = errorMessage,
        dismissAfterMillis = Delays.ERROR_TIMEOUT,
        onDismiss = onDismiss
    )

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
                        onClick = { reloadClicked = true },
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

                if (connecting || discovering) {
                    Spacer(modifier = Modifier.height(200.dp))
                    Box (modifier = Modifier.fillMaxSize()) {
                        Column (modifier = Modifier.align(Alignment.TopCenter)) {
                            CircularProgressIndicator(
                                color = MaterialTheme.colorScheme.primary,
                                strokeWidth = 6.dp,
                                modifier = Modifier
                                    .size(75.dp)
                                    .align(Alignment.CenterHorizontally)
                            )

                            Spacer(modifier = Modifier.height(10.dp))

                            Text(
                                text = if (connecting && BLEDeviceManager.selectedDevice != null) "Conectando a ${BLEDeviceManager.selectedDevice?.name}" else "Buscando Dispositivos de Audio",
                                style = MaterialTheme.typography.titleMedium,
                                textAlign = TextAlign.Center,
                                modifier = Modifier.fillMaxWidth()
                            )
                        }
                    }
                }
                else {
                    Spacer(modifier = Modifier.height(20.dp))

                    if (foundAudioDevices.isEmpty()) {
                        Text(
                            text = "No se encontraron dispositivos de audio disponibles",
                            style = MaterialTheme.typography.titleMedium,
                            textAlign = TextAlign.Center,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                    else {
                        LazyColumn {
                            items(foundAudioDevices) { device ->
                                DeviceCard(device.name) { selectedDevice = device }
                                Spacer(modifier = Modifier.height(8.dp))
                            }
                        }
                    }
                }
            }
            CommandButton(modifier = Modifier.align(Alignment.BottomCenter))
        }
    }
}

@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@Preview
@Composable
fun ScanningViewPreview () {
    ScanningView({}, {})
}
