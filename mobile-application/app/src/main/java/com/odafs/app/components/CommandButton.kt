package com.odafs.app.components

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.Intent
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
    view?.isHapticFeedbackEnabled = true
    val vibrator = context.getSystemService(Context.VIBRATOR_SERVICE) as Vibrator

    fun vibrate(duration: Long = 150) {
        vibrator.vibrate(VibrationEffect.createOneShot(duration, VibrationEffect.DEFAULT_AMPLITUDE))
    }

    val speechLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.StartActivityForResult()
    ) { result ->
        // Vibración que indica que el micrófono dejó de escuchar
        view?.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS)

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
            Toast.makeText(context, "No se pudo reconocer tu voz", Toast.LENGTH_SHORT).show()
        }
    }

    IconButton(
        onClick = {
            // Vibración que indica que el micrófono está escuchando
            vibrate(500)


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
