package com.odafs.app.components

import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.tooling.preview.Preview

@Composable
fun DeviceCard (name: String, onConnectClicked: () -> Unit) {
    Text(text = name)
}

@Preview
@Composable
fun DeviceCardPreview () {
    DeviceCard(name = "device 1", {})
}