package com.odafs.app.views

import android.Manifest
import android.os.Build
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
import com.odafs.app.ble.BLEController
import com.odafs.app.components.CommandButton
import com.odafs.app.components.TopBar
import kotlinx.coroutines.delay
import kotlin.math.roundToInt

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@Composable
fun ControlsView(deviceName: String, navigateToScanning: () -> Unit) {
    var disconnectClicked by remember { mutableStateOf(false) }

    LaunchedEffect(Unit) {
        while (true) {
            delay(2050)
            BLEController.healthCheck()
        }
    }

    LaunchedEffect(Unit) {
        while (true) {
            delay(2550)
            if (!BLEController.audioHealthCheck()) {
                navigateToScanning()
            }
        }
    }

    LaunchedEffect(disconnectClicked) {
        if (disconnectClicked) {
            if (BLEController.disconnectAudioDevice()) {
                disconnectClicked = false
                navigateToScanning()
            }
        }
    }

    Scaffold(
        topBar = { TopBar(title = deviceName) }
    ) { innerPadding ->
        Box(modifier = Modifier
            .fillMaxSize()
            .padding(
                top = innerPadding.calculateTopPadding(),
                bottom = 75.dp
            )
        ) {
            val surfaceHorizontalPadding = 25
            val surfaceVerticalPadding = 15
            Column {
                Surface (shadowElevation = 1.dp) {
                    Column (modifier = Modifier
                        .padding(
                            bottom = surfaceVerticalPadding.dp,
                            start = surfaceHorizontalPadding.dp,
                            end = surfaceHorizontalPadding.dp
                        )
                    ) {
                        Box(modifier = Modifier
                            .fillMaxWidth()
                            .padding(vertical = 50.dp)
                        ) {
                            var isPlaying by remember { mutableStateOf(false) }

                            val volumeButtonSize = 65
                            val volumeIconSize = 55

                            val playButtonSize = 90
                            val playIconSize = 80

                            IconButton(
                                onClick = {},
                                modifier = Modifier
                                    .align(Alignment.CenterStart)
                                    .background(
                                        color = MaterialTheme.colorScheme.surfaceContainer,
                                        shape = CircleShape
                                    )
                                    .size(volumeButtonSize.dp)
                            ) {
                                Icon(
                                    imageVector = Icons.AutoMirrored.Default.VolumeDown,
                                    contentDescription = "Botón para disminuir el volumen de la retroalimentación",
                                    tint = MaterialTheme.colorScheme.inverseSurface,
                                    modifier = Modifier.size(volumeIconSize.dp)
                                )
                            }

                            IconButton(
                                onClick = { isPlaying = !isPlaying },
                                modifier = Modifier
                                    .align(Alignment.Center)
                                    .background(
                                        color = MaterialTheme.colorScheme.primary,
                                        shape = CircleShape
                                    )
                                    .size(playButtonSize.dp)
                            ) {
                                Icon(
                                    imageVector = if (isPlaying) Icons.Filled.Pause else Icons.Default.PlayArrow,
                                    contentDescription = "Botón para pausar o reanudar la retroalimentación",
                                    tint = MaterialTheme.colorScheme.inverseSurface,
                                    modifier = Modifier.size(playIconSize.dp)
                                )
                            }

                            IconButton(
                                onClick = {},
                                modifier = Modifier
                                    .align(Alignment.CenterEnd)
                                    .background(
                                        color = MaterialTheme.colorScheme.surfaceContainer,
                                        shape = CircleShape
                                    )
                                    .size(volumeButtonSize.dp)
                            ) {
                                Icon(
                                    imageVector = Icons.AutoMirrored.Default.VolumeUp,
                                    contentDescription = "Botón para aumentar el volumen de la retroalimentación",
                                    tint = MaterialTheme.colorScheme.inverseSurface,
                                    modifier = Modifier.size(volumeIconSize.dp)
                                )
                            }
                        }

                        Column {
                            var volumeSliderValue by remember { mutableFloatStateOf(0.5f) }
                            Box (modifier = Modifier.fillMaxWidth()) {
                                Text(
                                    text = "Volumen",
                                    style = MaterialTheme.typography.bodyLarge,
                                    modifier = Modifier.align(Alignment.CenterStart)
                                )

                                Text(
                                    text = "${(100 * volumeSliderValue).roundToInt()}%",
                                    style = MaterialTheme.typography.bodyLarge,
                                    modifier = Modifier.align(Alignment.CenterEnd),
                                    color = Color.Gray
                                )
                            }

                            Spacer(modifier = Modifier.height(10.dp))

                            Slider(
                                value = volumeSliderValue,
                                onValueChange = {volumeSliderValue = it}
                            )
                        }
                    }
                }

                Spacer(modifier = Modifier.height(1.dp))

                Surface (shadowElevation = 1.dp) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(
                                vertical = surfaceVerticalPadding.dp,
                                horizontal = surfaceHorizontalPadding.dp
                            )
                    ) {
                        var checked by remember { mutableStateOf(true) }
                        Text(
                            text = "Modo",
                            style = MaterialTheme.typography.bodyLarge,
                            modifier = Modifier.align(Alignment.CenterStart)
                        )

                        Switch(
                            checked = checked,
                            onCheckedChange = { checked = it },
                            modifier = Modifier.align(Alignment.CenterEnd)
                        )
                    }
                }

                Spacer(modifier = Modifier.height(1.dp))

                Surface (shadowElevation = 1.dp) {
                    Column(
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(
                                vertical = surfaceVerticalPadding.dp,
                                horizontal = surfaceHorizontalPadding.dp
                            )
                    ) {
                        Button(
                            onClick = {},
                            shape = RoundedCornerShape(6.dp),
                            colors = ButtonDefaults.buttonColors(
                                containerColor = MaterialTheme.colorScheme.surfaceContainerHigh,
                                contentColor = MaterialTheme.colorScheme.inverseSurface
                            ),
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text(
                                text = "Reiniciar",
                                style = MaterialTheme.typography.bodyLarge,
                                fontWeight = FontWeight.Bold
                            )
                        }

                        Spacer(modifier = Modifier.height(surfaceVerticalPadding.dp))

                        Button(
                            onClick = { disconnectClicked = true },
                            shape = RoundedCornerShape(6.dp),
                            colors = ButtonDefaults.buttonColors(
                                containerColor = MaterialTheme.colorScheme.errorContainer,
                                contentColor = MaterialTheme.colorScheme.error
                            ),
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Text(
                                text = "Desconectar",
                                style = MaterialTheme.typography.bodyLarge,
                                fontWeight = FontWeight.Bold
                            )
                        }
                    }
                }
            }

            CommandButton(modifier = Modifier.align(Alignment.BottomCenter))
        }
    }
}

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
@Preview
@Composable
fun ControlsViewPreview() {
    ControlsView ("Device Name") { }
}
