package com.odafs.app.components

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Help
import androidx.compose.material.icons.automirrored.filled.Logout
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp

@Composable
fun TopBar (title: String) {
    Surface (shadowElevation = 1.dp) {
        Box (modifier = Modifier
            .fillMaxWidth()
            .padding(all = 15.dp)
        ) {
            IconButton(
                onClick = {},
                modifier = Modifier
                    .align(Alignment.CenterStart)
                    .size(45.dp)
            ) {
                Icon(
                    imageVector = Icons.AutoMirrored.Default.Logout,
                    contentDescription = "Botón para cerrar la aplicación",
                    modifier = Modifier.size(30.dp)
                )
            }

            Text(
                text = title,
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                maxLines = 1,
                modifier = Modifier.align(Alignment.Center)
            )

            IconButton(
                onClick = {},
                modifier = Modifier
                    .align(Alignment.CenterEnd)
                    .size(45.dp)
            ) {
                Icon(
                    imageVector = Icons.AutoMirrored.Default.Help,
                    contentDescription = "Botón para obtener ayuda sobre el uso de la aplicación",
                    modifier = Modifier.size(30.dp)
                )
            }
        }
    }
}

@Preview (showBackground = true)
@Composable
fun TopBarPreview() {
    TopBar(title = "title")
}
