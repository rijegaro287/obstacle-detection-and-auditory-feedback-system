package com.odafs.app.logic

import android.Manifest
import android.content.Context
import android.os.Build
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import com.odafs.app.ble.BLEClient
import com.odafs.app.ble.BLEClient.disconnectAudioDevice
import com.odafs.app.ble.BLEClient.scanForAudioDevices
import com.odafs.app.ble.BLEClient.setAudioVolume
import com.odafs.app.ble.BLEClient.setFeedbackMode
import com.odafs.app.ble.BLEClient.startAudioFeedback
import com.odafs.app.ble.BLEClient.stopAudioFeedback
import com.odafs.app.ble.FeedbackModes.NON_VERBAL_FEEDBACK
import com.odafs.app.ble.FeedbackModes.VERBAL_FEEDBACK
import kotlinx.coroutines.flow.first
import java.text.Normalizer
import java.util.Locale

@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
suspend fun processCommand(context: Context, command: String) {
    // Pasar a minúsculas y quitar tildes
    val lowerCommand = Normalizer.normalize(
        command.lowercase(Locale.getDefault()),
        Normalizer.Form.NFD
    ).replace("\\p{InCombiningDiacriticalMarks}+".toRegex(), "")

    when {
        // --- Configurar volumen ---
        lowerCommand.contains("configurar volumen") -> {
            val percentage = extractPercentage(lowerCommand)
            if (percentage != null) {
                val volume = (percentage.toFloat() / 100f)
                setAudioVolume(volume)
                Toast.makeText(
                    context,
                    "🔊 Volumen configurado al $percentage%",
                    Toast.LENGTH_SHORT
                ).show()
            } else {
                Toast.makeText(
                    context,
                    "❓ No entendí el porcentaje en: $command",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }

        // --- Pausar reproduccion de audio ---
        lowerCommand.contains("pausar") -> {
            Toast.makeText(context, "🔵 Pausando reproducción de audio...", Toast.LENGTH_SHORT).show()
            stopAudioFeedback()
        }

        // --- Reanudar reproduccion de audio ---
        lowerCommand.contains("reanudar") -> {
            Toast.makeText(context, "🔵 Reanudando reproducción de audio...", Toast.LENGTH_SHORT).show()
            startAudioFeedback()

        }

        // --- Apagar sistema ---
        lowerCommand.contains("apagar") -> {
            Toast.makeText(context, "🔵 Apagando el sistema...", Toast.LENGTH_SHORT).show()
        }

        // --- Escanear dispositivos de audio ---
        lowerCommand.contains("escanear dispositivos de audio") -> {
            Toast.makeText(context, "🔵 Escaneando dispositivos de audio...", Toast.LENGTH_SHORT).show()
            scanForAudioDevices()
        }
        
        // --- Mostrar dispositivos de audio encontrados ---
        lowerCommand.contains("mostrar dispositivos de audio") -> {
            // Obtener los dispositivos encontrados
            val devices = BLEClient.GATTConnection.foundAudioDevices.first()

            if (devices.isNotEmpty()) {
                val deviceNames = devices.joinToString(", ") { it.name }
                Toast.makeText(
                    context,
                    "📡 Dispositivos encontrados: $deviceNames",
                    Toast.LENGTH_LONG
                ).show()
            } else {
                Toast.makeText(
                    context,
                    "⚠️ No se encontraron dispositivos de audio",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }

        // --- Conectar dispositivo de audio ---
        lowerCommand.contains("conectar dispositivo") -> {
            val regex = Regex("""\d+""")
            val match = regex.find(lowerCommand)

            if (match != null) {
                val indexSpoken = match.value.toInt()
                val devices = BLEClient.GATTConnection.foundAudioDevices.value

                if (devices.isNotEmpty() && indexSpoken in 1..devices.size) {
                    val device = devices[indexSpoken - 1]

                    // lanzar corrutina para conectar (si pairAndConnectAudioDevice es suspend)
                    val connected = BLEClient.pairAndConnectAudioDevice(device)
                    if (connected) {
                        Toast.makeText(
                            context,
                            "✅ Conectado a ${device.name}",
                            Toast.LENGTH_SHORT
                        ).show()
                    } else {
                        Toast.makeText(
                            context,
                            "❌ No se pudo conectar a ${device.name}",
                            Toast.LENGTH_SHORT
                        ).show()
                    }


                } else {
                    Toast.makeText(
                        context,
                        "⚠️ Número de dispositivo inválido. Solo hay ${devices.size} disponibles.",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            } else {
                Toast.makeText(
                    context,
                    "⚠️ No se detectó número de dispositivo en el comando.",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }



        // --- Desconectar dispositivo de audio ---
        lowerCommand.contains("desconectar dispositivo de audio") -> {
            Toast.makeText(context, "🔵 Desconectando dispositivo de audio...", Toast.LENGTH_SHORT).show()
            disconnectAudioDevice()

        }

        // --- Tipo de retroalimentacion NO verbal ---
        lowerCommand.contains("retroalimentacion no verbal") -> {
            Toast.makeText(context, "🔵 Cambiando a retroalimentación no verbal...", Toast.LENGTH_SHORT).show()
            setFeedbackMode(NON_VERBAL_FEEDBACK)
        }

        // --- Tipo de retroalimentacion verbal ---
        lowerCommand.contains("retroalimentacion verbal") -> {
            Toast.makeText(context, "🔵 Cambiando a retroalimentación verbal...", Toast.LENGTH_SHORT).show()
            setFeedbackMode(VERBAL_FEEDBACK)
        }

        // --- Ayuda ---
        lowerCommand.contains("ayuda") -> {
            Toast.makeText(context, "🔵 Información de ayuda", Toast.LENGTH_SHORT).show()
        }

        else -> {
            Toast.makeText(context, "❓ No entendí el comando: $command", Toast.LENGTH_SHORT).show()
        }
    }
}

/**
 * Extrae el primer número encontrado en el texto y lo interpreta como porcentaje.
 */
fun extractPercentage(text: String): Int? {
    val regex = Regex("(\\d+)")
    val match = regex.find(text)
    return match?.value?.toIntOrNull()?.coerceIn(0, 100)
}

