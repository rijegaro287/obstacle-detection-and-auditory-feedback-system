package com.odafs.app.components

import android.widget.Space
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.Mic
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp

@Composable
fun DeviceCard (name: String, onConnectClicked: () -> Unit) {
    Surface (shadowElevation = 1.dp) {
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .padding(all = 10.dp)
        ) {
            Row(modifier = Modifier.align(Alignment.CenterStart)) {
                Box(
                    modifier = Modifier
                        .size(45.dp)
                        .background(
                            color = MaterialTheme.colorScheme.surfaceContainer,
                            shape = CircleShape
                        )
                ) {
                    Icon(
                        imageVector = Icons.Default.Bluetooth,
                        contentDescription = "Ícono de Bluetooth",
                        tint = MaterialTheme.colorScheme.primary,
                        modifier = Modifier
                            .size(35.dp)
                            .align(Alignment.Center)
                    )
                }


                Spacer(modifier = Modifier.width(12.dp))

                Column (modifier = Modifier.align(Alignment.CenterVertically)) {
                    Text(
                        text = name,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )

                    Spacer(modifier = Modifier.height(2.dp))

                    Text(
                        text = "Listo para conectar",
                        style = MaterialTheme.typography.labelSmall,
                        color = Color.Gray
                    )
                }
            }

            Button(
                onClick = { onConnectClicked() },
                shape = RoundedCornerShape(6.dp),
                contentPadding = PaddingValues(horizontal = 15.dp),
                modifier = Modifier
                    .align(Alignment.CenterEnd)
                    .height(32.dp)

            ) {
                Text(
                    text = "Conectar",
                    style = MaterialTheme.typography.labelMedium,
                )
            }
        }
    }
}

@Preview(showBackground = true)
@Composable
fun DeviceCardPreview () {
    DeviceCard(name = "device 1", {})
}
