package com.odafs.app

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.toRoute
import kotlinx.serialization.Serializable
import com.odafs.app.views.ConnectingView
import com.odafs.app.views.ControlsView
import com.odafs.app.views.ScanningView

@Serializable
data object Connecting

@Serializable
data object Scanning

@Serializable
data class Controls(val deviceName: String)

@Composable
fun NavigationController() {
    val navController = rememberNavController()
    Scaffold { innerPadding ->
        NavHost(
            navController = navController,
            startDestination = Connecting,
            modifier = Modifier.padding(innerPadding)
        ) {
            composable<Connecting> {
                BackHandler (enabled = true) {  }
                ConnectingView (
                    navigateToScanning = { navController.navigate(Scanning) }
                )
            }

            composable<Scanning> {
                BackHandler (enabled = true) {  }
                ScanningView (
                    navigateToConnecting = { navController.navigate(Connecting) },
                    navigateToControls = { deviceName -> navController.navigate(Controls(deviceName = deviceName)) }
                )
            }

            composable<Controls> { backStackEntry ->
                val controls = backStackEntry.toRoute<Controls>()
                // BackHandler (enabled = true) {  }
                ControlsView (deviceName = controls.deviceName) {
                    navController.navigate(Scanning) {
                        popUpTo(Connecting) { inclusive = true }
                    }
                }
            }
        }
    }
}
