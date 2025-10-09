package com.odafs.app.components

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.media.AudioManager
import android.media.ToneGenerator
import android.os.Build
import android.speech.RecognizerIntent
import android.speech.tts.TextToSpeech
import android.view.HapticFeedbackConstants
import android.widget.Toast
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Mic
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.odafs.app.logic.processCommand
import kotlinx.coroutines.launch
import java.util.Locale
import android.os.VibrationEffect
import android.os.Vibrator

private var textToSpeech: TextToSpeech? = null

@Composable
@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(allOf = [Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.VIBRATE])
fun CommandButton(modifier: Modifier = Modifier) {
    val context = LocalContext.current
    val speechText = remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()
    val view = (context as? Activity)?.window?.decorView

    val speechLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.StartActivityForResult()
    ) { result ->
        // Vibración que indica que el micrófono dejó de escuchar
        vibrate(context, HapticType.DOUBLE_TAP)
        //playTone(ToneType.STOP_LISTEN)

        if (result.resultCode == Activity.RESULT_OK) {
            val data = result.data
            val matches = data?.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS)
            val spoken = matches?.get(0) ?: ""
            speechText.value = spoken
            Toast.makeText(context, "Dijiste: $spoken", Toast.LENGTH_LONG).show()
            scope.launch {
                processCommand(context, spoken)
            }
        } else {
            vibrate(context, HapticType.ERROR)
            playTone(ToneType.ERROR)
            Toast.makeText(context, "No se pudo reconocer tu voz", Toast.LENGTH_SHORT).show()
        }
    }

    IconButton(
        onClick = {
            // Vibración que indica que el micrófono está escuchando
            vibrate(context, HapticType.TAP)
            playTone(ToneType.START_LISTEN)


            val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
                putExtra(
                    RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                    RecognizerIntent.LANGUAGE_MODEL_FREE_FORM
                )
                putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.getDefault())
                putExtra(RecognizerIntent.EXTRA_PROMPT, "Habla ahora…")
            }
            speechLauncher.launch(intent)
        },
        modifier = modifier
            .background(
                color = MaterialTheme.colorScheme.primaryContainer,
                shape = CircleShape
            )
            .size(120.dp)
    ) {
        Icon(
            imageVector = Icons.Default.Mic,
            contentDescription = "Botón para ingresar un comando de voz",
            tint = MaterialTheme.colorScheme.primary,
            modifier = Modifier.size(100.dp)
        )
    }
}

enum class HapticType { SUCCESS, ERROR, WARNING, INFO, TAP, LONG_PRESS, DOUBLE_TAP }
@RequiresApi(Build.VERSION_CODES.O)
fun vibrate(context: Context, type: HapticType) {
    val vibrator = context.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
    val pattern = when (type) {
        HapticType.SUCCESS -> longArrayOf(0, 50, 50, 50)
        HapticType.ERROR -> longArrayOf(0, 100, 50, 200)
        HapticType.WARNING -> longArrayOf(0, 80, 40, 80, 40, 80)
        HapticType.INFO -> longArrayOf(0, 150)
        HapticType.TAP -> longArrayOf(0, 30)
        HapticType.LONG_PRESS -> longArrayOf(0, 250)
        HapticType.DOUBLE_TAP -> longArrayOf(0, 40, 60, 40)
    }
    vibrator.vibrate(VibrationEffect.createWaveform(pattern, -1))
}

enum class ToneType { SUCCESS, ERROR, START_LISTEN, STOP_LISTEN, NOTIFICATION, DISCONNECT, TAP }

fun playTone(type: ToneType) {
    val toneGen = ToneGenerator(AudioManager.STREAM_MUSIC, 100)

    val (tone, duration) = when (type) {
        ToneType.SUCCESS -> ToneGenerator.TONE_PROP_ACK to 150
        ToneType.ERROR -> ToneGenerator.TONE_PROP_NACK to 200
        ToneType.START_LISTEN -> ToneGenerator.TONE_PROP_BEEP to 120
        ToneType.STOP_LISTEN -> ToneGenerator.TONE_PROP_BEEP2 to 150
        ToneType.NOTIFICATION -> ToneGenerator.TONE_CDMA_ALERT_CALL_GUARD to 250
        ToneType.DISCONNECT -> ToneGenerator.TONE_PROP_BEEP2 to 200
        ToneType.TAP -> ToneGenerator.TONE_PROP_BEEP to 80
    }

    toneGen.startTone(tone, duration)
}


fun initTTS(context: Context) {
    textToSpeech = TextToSpeech(context) { status ->
        if (status == TextToSpeech.SUCCESS) {
            textToSpeech?.language = Locale.getDefault()
        }
    }
}

fun speak(text: String) {
    textToSpeech?.speak(text, TextToSpeech.QUEUE_FLUSH, null, null)
}



@RequiresApi(Build.VERSION_CODES.TIRAMISU)
@RequiresPermission(allOf = [Manifest.permission.BLUETOOTH_CONNECT, Manifest.permission.VIBRATE])
@Preview
@Composable
fun CommandButtonPreview() {
    CommandButton(modifier = Modifier)
}
