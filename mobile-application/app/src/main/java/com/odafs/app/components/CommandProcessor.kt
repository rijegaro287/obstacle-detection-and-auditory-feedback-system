package com.odafs.app.logic

import android.Manifest
import android.content.Context
import android.os.Build
import android.speech.tts.TextToSpeech
import android.view.HapticFeedbackConstants
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.compose.ui.platform.LocalView
import com.odafs.app.ble.BLEClient
import com.odafs.app.ble.BLEClient.disconnectAudioDevice
import com.odafs.app.ble.BLEClient.scanForAudioDevices
import com.odafs.app.ble.BLEClient.setAudioVolume
import com.odafs.app.ble.BLEClient.setFeedbackMode
import com.odafs.app.ble.BLEClient.startAudioFeedback
import com.odafs.app.ble.BLEClient.stopAudioFeedback
import com.odafs.app.ble.FeedbackModes.NON_VERBAL_FEEDBACK
import com.odafs.app.ble.FeedbackModes.VERBAL_FEEDBACK
import com.odafs.app.components.HapticType
import com.odafs.app.components.ToneType
import com.odafs.app.components.playTone
import com.odafs.app.components.speak
import com.odafs.app.components.vibrate
import kotlinx.coroutines.flow.first
import java.text.Normalizer
import java.util.Locale
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
suspend fun processCommand(context: Context, command: String) {
    var isSuccess = true
    var isCommandValid = true
    var message = ""

    // Pasar a minúsculas y quitar tildes
    val lowerCommand = Normalizer.normalize(
        command.lowercase(Locale.forLanguageTag("es-419")),
        Normalizer.Form.NFD
    ).replace("\\p{InCombiningDiacriticalMarks}+".toRegex(), "")

    when {
        // --- Configurar volumen ---
        lowerCommand.contains("configurar volumen" ) || lowerCommand.contains("configurar el volumen") -> {
            val percentage = extractPercentage(lowerCommand)
            if (percentage != null) {
                val volume = (percentage.toFloat() / 100f)
                isSuccess = setAudioVolume(volume)
                Toast.makeText(
                    context,
                    "🔊 Volumen configurado al $percentage%",
                    Toast.LENGTH_SHORT
                ).show()
                message = "Volumen configurado al $percentage%"
            } else {
                isCommandValid = false
                message = "Lo siento, no entendí el porcentaje de volumen."
                Toast.makeText(
                    context,
                    "❓ No entendí el porcentaje en: $command",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }

        // --- Pausar reproduccion de audio ---
        lowerCommand.contains("pausar") -> {
            message = "Pausando reproducción de audio"
            Toast.makeText(context, "🔵 Pausando reproducción de audio...", Toast.LENGTH_SHORT).show()
            isSuccess = stopAudioFeedback()
        }

        // --- Reanudar reproduccion de audio ---
        lowerCommand.contains("reanudar") -> {
            message = "Reanundando reproducción de audio"
            Toast.makeText(context, "🔵 Reanudando reproducción de audio...", Toast.LENGTH_SHORT).show()
            isSuccess = startAudioFeedback()

        }

        // --- Apagar sistema ---
        /** lowerCommand.contains("apagar") -> {
            Toast.makeText(context, "🔵 Apagando el sistema...", Toast.LENGTH_SHORT).show()
        }**/

        // --- Escanear dispositivos de audio ---
        lowerCommand.contains("escanear dispositivos de audio") -> {
            message = "Escaneando dispositivos de audio"
            Toast.makeText(context, "🔵 Escaneando dispositivos de audio...", Toast.LENGTH_SHORT).show()
            isSuccess = scanForAudioDevices()
        }

        // --- Mostrar dispositivos de audio encontrados ---
        lowerCommand.contains("mostrar dispositivos de audio") -> {
            // Obtener los dispositivos encontrados
            val devices = BLEClient.GATTConnection.foundAudioDevices.first()

            if (devices.isNotEmpty()) {
                val deviceList = devices.mapIndexed { index, device ->
                    "Dispositivo ${index + 1}: ${device.name}"
                }.joinToString(", ")

                message = devices.mapIndexed { index, device ->
                    "Dispositivo ${index + 1}: ${device.name}"
                }.joinToString(", ")

                Toast.makeText(
                    context,
                    "📡 Dispositivos encontrados: $deviceList",
                    Toast.LENGTH_LONG
                ).show()

            } else {
                message = "No se encontraron dispositivos de audio, intente escanear dispositivos de audio nuevamente"

                Toast.makeText(
                    context,
                    "⚠️ No se encontraron dispositivos de audio",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }

        // --- Conectar dispositivo de audio ---
        lowerCommand.contains("conectar dispositivo de audio") ||
            lowerCommand.contains("conectarse al dispositivo de audio")-> {
            val regex = Regex("""\d+""")
            val match = regex.find(lowerCommand)

            if (match != null) {
                val indexSpoken = match.value.toInt()
                val devices = BLEClient.GATTConnection.foundAudioDevices.value

                if (devices.isNotEmpty() && indexSpoken in 1..devices.size) {
                    val device = devices[indexSpoken - 1]

                    // lanzar corrutina para conectar (si pairAndConnectAudioDevice es suspend)
                    val connected = BLEClient.pairAndConnectAudioDevice(device)
                    isSuccess = connected
                    if (connected) {
                        message = "Conectado a ${device.name}"
                        Toast.makeText(
                            context,
                            "✅ Conectado a ${device.name}",
                            Toast.LENGTH_SHORT
                        ).show()
                    } else {
                        message = "No fue posible conectarse a ${device.name}"
                        Toast.makeText(
                            context,
                            "❌ No se pudo conectar a ${device.name}",
                            Toast.LENGTH_SHORT
                        ).show()
                    }

                } else {
                    isCommandValid = false
                    message = "Número de dispositivo inválido. Solo hay ${devices.size} disponibles."
                    Toast.makeText(
                        context,
                        "⚠️ Número de dispositivo inválido. Solo hay ${devices.size} disponibles.",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            } else {
                isCommandValid = false
                message = "No entendí el número de dispositivo seleccionado."
                Toast.makeText(
                    context,
                    "⚠️ No se detectó número de dispositivo en el comando.",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }

        // --- Desconectar dispositivo de audio ---
        lowerCommand.contains("desconectar dispositivo de audio") -> {
            message = "Desconectando dispositivo de audio"
            Toast.makeText(context, "🔵 Desconectando dispositivo de audio...", Toast.LENGTH_SHORT).show()
            isSuccess = disconnectAudioDevice()
        }

        // --- Tipo de retroalimentacion NO verbal ---
        lowerCommand.contains("retroalimentacion no verbal") -> {
            message = "Cambiando a retroalimentación no verbal"
            Toast.makeText(context, "🔵 Cambiando a retroalimentación no verbal...", Toast.LENGTH_SHORT).show()
            isSuccess = setFeedbackMode(NON_VERBAL_FEEDBACK)
        }

        // --- Tipo de retroalimentacion verbal ---
        lowerCommand.contains("retroalimentacion verbal") -> {
            message = "Cambiando a retroalimentación verbal"
            Toast.makeText(context, "🔵 Cambiando a retroalimentación verbal...", Toast.LENGTH_SHORT).show()
            isSuccess = setFeedbackMode(VERBAL_FEEDBACK)
        }

        // --- Ayuda ---
        lowerCommand.contains("ayuda") -> {
            Toast.makeText(context, "🔵 Información de ayuda", Toast.LENGTH_SHORT).show()
            message = "Te damos la bienvenida a nuestra aplicación de control, puedes interactuar por medio " +
                    "de comandos de voz al presionar el centro de la pantalla, escucharás un tono que indica que " +
                    "el microfono esta activo. Puedes configurar tu herramienta de asistencia utilizando frases " +
                    "como: reanudar, pausar, configurar volumen, retroalimentación verbal, retroalimentación no " +
                    "verbal, escanear dispositivos de audio, mostrar dispositivos de audio, conectar dispositivo " +
                    "de audio, desconectar dispositivo de audio. En caso de que ocurra algún error te lo haremos " +
                    "saber mediante retroalimentacion háptica y auditiva. Para mas información sobre el uso de la " +
                    "aplicación puedes acceder al manual de usuario. "
        }

        else -> {
            message = "Lo siento, no entendí el comando."
            Toast.makeText(context, "❓ No entendí el comando: $command", Toast.LENGTH_SHORT).show()
        }
    }

    speak(message)

    if (isCommandValid) {
        if (isSuccess) {
            vibrate(context, HapticType.SUCCESS)
            playTone(ToneType.SUCCESS)
        } else {
            vibrate(context, HapticType.ERROR)
            playTone(ToneType.ERROR)
            speak("Lo siento, la operación no se pudo completar. Por favor inténtalo de nuevo.")
        }
    } else {
        vibrate(context, HapticType.WARNING)
        playTone(ToneType.WARNING)
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

