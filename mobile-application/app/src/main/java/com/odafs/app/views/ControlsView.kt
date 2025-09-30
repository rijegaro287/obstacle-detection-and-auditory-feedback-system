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
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.VolumeDown
import androidx.compose.material.icons.automirrored.filled.VolumeUp
import androidx.compose.material.icons.filled.Pause
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Slider
import androidx.compose.material3.Surface
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.odafs.app.ble.BTAudioDevice
import com.odafs.app.ble.Delays
import com.odafs.app.ble.FeedbackModes
import com.odafs.app.components.AutoDismissDialog
import com.odafs.app.components.CommandButton
import com.odafs.app.components.TopBar
import kotlinx.coroutines.delay
import java.lang.Math.clamp
import kotlin.math.roundToInt

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@Composable
fun ControlsView(
    deviceName: String,
    navigateToConnecting: () -> Unit,
    navigateToScanning: () -> Unit
) {
//    var connected by remember { mutableStateOf(false) }
//    connected = BLEController.connected.collectAsState().value
//
//    var connectedAudioDevice by remember { mutableStateOf<BTAudioDevice?>(null) }
//    connectedAudioDevice = BLEController.connectedAudioDevice.collectAsState().value
//
//    var isPlaying by remember { mutableStateOf(false) }
//    var disconnectClicked by remember { mutableStateOf(false) }
//    var volumeValue by remember { mutableFloatStateOf(0.5f) }
//    var switchChecked by remember { mutableStateOf(false) }
//
//    var showError by remember { mutableStateOf(false) }
//    var errorTitle by remember { mutableStateOf("") }
//    var errorMessage by remember { mutableStateOf("") }
//    var onDismiss : () -> Unit by remember { mutableStateOf({}) }
//    val errorTimeout = 5000L
//
//    LaunchedEffect(connected) {
//        if (!connected) {
//            errorTitle = "Conexión perdida"
//            errorMessage = "Se perdió la conexión con el dispositivo de procesamiento"
//            showError = true
//            onDismiss = {
//                showError = false
//                navigateToConnecting()
//            }
//        }
//    }
//
//    LaunchedEffect(connectedAudioDevice) {
//        if (connectedAudioDevice == null) {
//            errorTitle = "Conexión perdida"
//            errorMessage = "Se perdió la conexión con el dispositivo de audio"
//            showError = true
//            onDismiss = {
//                showError = false
//                navigateToScanning()
//            }
//        }
//    }
//
//    LaunchedEffect(Unit) {
//        while (true) {
//            delay(Delays.HEALTH_CHECK_DELAY)
//            BLEController.healthCheck()
//        }
//    }
//
//    LaunchedEffect(Unit) {
//        while (true) {
//            delay(Delays.AUDIO_HEALTH_CHECK_DELAY)
//            BLEController.audioHealthCheck()
//        }
//    }
//
//    LaunchedEffect(disconnectClicked) {
//        while (true) {
//            if (disconnectClicked) {
//                Log.d("BLE Controller", "Disconnecting from audio device")
//                val disconnectSuccess = BLEController.disconnectAudioDevice()
//                if (disconnectSuccess) {
//                    disconnectClicked = false
//                    navigateToScanning()
//                }
//            }
//            else {
//                break
//            }
//            delay(Delays.MISC_DELAY)
//        }
//    }
//
//    LaunchedEffect(isPlaying) {
//        while (true) {
//            Log.d("BLE Controller", "Setting feedback state to $isPlaying")
//            val stateChanged = if (isPlaying) {
//                BLEController.startAudioFeedback()
//            }
//            else {
//                BLEController.stopAudioFeedback()
//            }
//            if (stateChanged) break
//            delay(Delays.MISC_DELAY)
//        }
//    }
//
//    LaunchedEffect(volumeValue) {
//        while (true) {
//            Log.d("BLE Controller", "Setting volume to $volumeValue")
//            val volumeInt = (100 * volumeValue).toInt()
//            val volumeChanged = BLEController.setAudioVolume(volumeInt)
//            if (volumeChanged) break
//            delay(Delays.MISC_DELAY)
//        }
//    }
//
//    LaunchedEffect(switchChecked) {
//        while (true) {
//            Log.d("BLE Controller", "Setting feedback mode to $switchChecked")
//            val feedbackModeChanged = if (switchChecked) {
//                BLEController.setFeedbackMode(FeedbackModes.VERBAL_FEEDBACK)
//            }
//            else {
//                BLEController.setFeedbackMode(FeedbackModes.NON_VERBAL_FEEDBACK)
//            }
//            if (feedbackModeChanged) break
//            delay(Delays.MISC_DELAY)
//        }
//    }
//
//    AutoDismissDialog(
//        visible = showError,
//        title = errorTitle,
//        message = errorMessage,
//        dismissAfterMillis = errorTimeout,
//        onDismiss = onDismiss
//    )
//
//    Scaffold(
//        topBar = { TopBar(title = deviceName) }
//    ) { innerPadding ->
//        Box(modifier = Modifier
//            .fillMaxSize()
//            .padding(
//                top = innerPadding.calculateTopPadding(),
//                bottom = 75.dp
//            )
//        ) {
//            val surfaceHorizontalPadding = 25
//            val surfaceVerticalPadding = 15
//            Column {
//                Surface (shadowElevation = 1.dp) {
//                    Column (modifier = Modifier
//                        .padding(
//                            bottom = surfaceVerticalPadding.dp,
//                            start = surfaceHorizontalPadding.dp,
//                            end = surfaceHorizontalPadding.dp
//                        )
//                    ) {
//                        Box(modifier = Modifier
//                            .fillMaxWidth()
//                            .padding(vertical = 50.dp)
//                        ) {
//                            val volumeStep = 5f / 100f
//                            val volumeButtonSize = 65
//                            val volumeIconSize = 55
//
//                            val playButtonSize = 90
//                            val playIconSize = 80
//
//                            IconButton(
//                                onClick = { volumeValue = clamp(volumeValue - volumeStep, 0f, 1.0f) },
//                                modifier = Modifier
//                                    .align(Alignment.CenterStart)
//                                    .background(
//                                        color = MaterialTheme.colorScheme.surfaceContainer,
//                                        shape = CircleShape
//                                    )
//                                    .size(volumeButtonSize.dp)
//                            ) {
//                                Icon(
//                                    imageVector = Icons.AutoMirrored.Default.VolumeDown,
//                                    contentDescription = "Botón para disminuir el volumen de la retroalimentación",
//                                    tint = MaterialTheme.colorScheme.inverseSurface,
//                                    modifier = Modifier.size(volumeIconSize.dp)
//                                )
//                            }
//
//                            IconButton(
//                                onClick = { isPlaying = !isPlaying },
//                                modifier = Modifier
//                                    .align(Alignment.Center)
//                                    .background(
//                                        color = MaterialTheme.colorScheme.primary,
//                                        shape = CircleShape
//                                    )
//                                    .size(playButtonSize.dp)
//                            ) {
//                                Icon(
//                                    imageVector = if (isPlaying) Icons.Filled.Pause else Icons.Default.PlayArrow,
//                                    contentDescription = "Botón para pausar o reanudar la retroalimentación",
//                                    tint = MaterialTheme.colorScheme.inverseSurface,
//                                    modifier = Modifier.size(playIconSize.dp)
//                                )
//                            }
//
//                            IconButton(
//                                onClick = { volumeValue = clamp(volumeValue + volumeStep, 0f, 1.0f) },
//                                modifier = Modifier
//                                    .align(Alignment.CenterEnd)
//                                    .background(
//                                        color = MaterialTheme.colorScheme.surfaceContainer,
//                                        shape = CircleShape
//                                    )
//                                    .size(volumeButtonSize.dp)
//                            ) {
//                                Icon(
//                                    imageVector = Icons.AutoMirrored.Default.VolumeUp,
//                                    contentDescription = "Botón para aumentar el volumen de la retroalimentación",
//                                    tint = MaterialTheme.colorScheme.inverseSurface,
//                                    modifier = Modifier.size(volumeIconSize.dp)
//                                )
//                            }
//                        }
//
//                        Column {
//                            Box (modifier = Modifier.fillMaxWidth()) {
//                                Text(
//                                    text = "Volumen",
//                                    style = MaterialTheme.typography.bodyLarge,
//                                    modifier = Modifier.align(Alignment.CenterStart)
//                                )
//
//                                Text(
//                                    text = "${(100 * volumeValue).roundToInt()}%",
//                                    style = MaterialTheme.typography.bodyLarge,
//                                    modifier = Modifier.align(Alignment.CenterEnd),
//                                    color = Color.Gray
//                                )
//                            }
//
//                            Spacer(modifier = Modifier.height(10.dp))
//
//                            Slider(
//                                value = volumeValue,
//                                onValueChange = {volumeValue = it}
//                            )
//                        }
//                    }
//                }
//
//                Spacer(modifier = Modifier.height(1.dp))
//
//                Surface (shadowElevation = 1.dp) {
//                    Box(
//                        modifier = Modifier
//                            .fillMaxWidth()
//                            .padding(
//                                vertical = surfaceVerticalPadding.dp,
//                                horizontal = surfaceHorizontalPadding.dp
//                            )
//                    ) {
//                        Text(
//                            text = "Modo Verbal",
//                            style = MaterialTheme.typography.bodyLarge,
//                            modifier = Modifier.align(Alignment.CenterStart)
//                        )
//
//                        Switch(
//                            checked = switchChecked,
//                            onCheckedChange = { switchChecked = it },
//                            modifier = Modifier.align(Alignment.CenterEnd)
//                        )
//                    }
//                }
//
//                Spacer(modifier = Modifier.height(1.dp))
//
//                Surface (shadowElevation = 1.dp) {
//                    Column(
//                        modifier = Modifier
//                            .fillMaxWidth()
//                            .padding(
//                                vertical = surfaceVerticalPadding.dp,
//                                horizontal = surfaceHorizontalPadding.dp
//                            )
//                    ) {
//                        Button(
//                            onClick = { disconnectClicked = true },
//                            shape = RoundedCornerShape(6.dp),
//                            colors = ButtonDefaults.buttonColors(
//                                containerColor = MaterialTheme.colorScheme.errorContainer,
//                                contentColor = MaterialTheme.colorScheme.error
//                            ),
//                            modifier = Modifier.fillMaxWidth()
//                        ) {
//                            Text(
//                                text = "Desconectar",
//                                style = MaterialTheme.typography.bodyLarge,
//                                fontWeight = FontWeight.Bold
//                            )
//                        }
//                    }
//                }
//            }
//
//            CommandButton(modifier = Modifier.align(Alignment.BottomCenter))
//        }
//    }
}
//
//@RequiresApi(Build.VERSION_CODES.TIRAMISU)
//@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
//@Preview
//@Composable
//fun ControlsViewPreview() {
//    ControlsView (
//        deviceName = "Device Name",
//        navigateToConnecting = { },
//        navigateToScanning = { }
//    )
//}
