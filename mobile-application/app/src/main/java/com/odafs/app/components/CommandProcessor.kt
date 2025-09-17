package com.odafs.app.logic

import android.content.Context
import android.media.AudioManager
import android.widget.Toast
import java.text.Normalizer
import java.util.Locale

fun processCommand(context: Context, command: String) {
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
                setVolume(context, percentage)
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
        }

        // --- Reanudar reproduccion de audio ---
        lowerCommand.contains("reanudar") -> {
            Toast.makeText(context, "🔵 Reanudando reproducción de audio...", Toast.LENGTH_SHORT).show()
        }

        // --- Apagar sistema ---
        lowerCommand.contains("apagar") -> {
            Toast.makeText(context, "🔵 Apagando el sistema...", Toast.LENGTH_SHORT).show()
        }

        // --- Reiniciar sistema ---
        lowerCommand.contains("reiniciar") -> {
            Toast.makeText(context, "🔵 Reiniciando sistema...", Toast.LENGTH_SHORT).show()
        }

        // --- Desconectar dispositivo de audio ---
        lowerCommand.contains("desconectar dispositivo de audio") -> {
            Toast.makeText(context, "🔵 Desconectando dispositivo de audio...", Toast.LENGTH_SHORT).show()
        }

        // --- Tipo de retroalimentacion NO verbal ---
        lowerCommand.contains("retroalimentacion no verbal") -> {
            Toast.makeText(context, "🔵 Cambiando a retroalimentación no verbal...", Toast.LENGTH_SHORT).show()
        }

        // --- Tipo de retroalimentacion verbal ---
        lowerCommand.contains("retroalimentacion verbal") -> {
            Toast.makeText(context, "🔵 Cambiando a retroalimentación verbal...", Toast.LENGTH_SHORT).show()
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

/**
 * Ajusta el volumen de la retroalimentación.
 */
fun setVolume(context: Context, percentage: Int) {

}
