package com.odafs.app.components

import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import kotlinx.coroutines.delay

@Composable
fun AutoDismissDialog(
    visible: Boolean,
    title: String,
    message: String,
    dismissAfterMillis: Long,
    onDismiss: () -> Unit
) {
    LaunchedEffect(key1 = visible) {
        if (visible) {
            delay(dismissAfterMillis)
            onDismiss()
        }
    }

    if (!visible) return

    AlertDialog(
        onDismissRequest = {
            onDismiss()
        },
        title = {
            Text(text = title)
        },
        text = {
            Text(text = message)
        },
        confirmButton = {
            TextButton(onClick = { onDismiss() }) {
                Text("Aceptar")
            }
        }
    )
}