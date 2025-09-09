package com.odafs.app

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.layout.Column
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import kotlinx.serialization.Serializable
import com.odafs.app.views.ConnectingView
import com.odafs.app.views.ScanningView

@Serializable
data object Connecting

@Serializable
data object Scanning

@Serializable
data object Controls

@Composable
fun ControlsView(navigateToScanning: () -> Unit) {
    Column {
        Text(text = "Controls")

        Button(onClick = { navigateToScanning() }) {
            Text(text = "Go to Scanning")
        }
    }
}

@Composable
fun NavigationController() {
    val navController = rememberNavController()
    NavHost(navController = navController, startDestination = Scanning) {
        composable<Connecting> {
            BackHandler (enabled = true) {  }
            ConnectingView { navController.navigate(Scanning) }
        }

        composable<Scanning> {
            BackHandler (enabled = true) {  }
            ScanningView { navController.navigate(Controls) }
        }

        composable<Controls> {
            BackHandler (enabled = true) {  }
            ControlsView {
                navController.navigate(Scanning) {
                    popUpTo(Connecting) { inclusive = true }
                }
            }
        }
    }
}
