/*
 * Copyright 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.platform.connectivity.bluetooth.ble

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.os.Build
import android.os.ParcelUuid
import android.util.Log
import androidx.annotation.RequiresApi
import androidx.annotation.RequiresPermission
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.rounded.Refresh
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalLifecycleOwner
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.LifecycleOwner
import kotlinx.coroutines.delay

/*
 * Copyright 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.content.Intent
import android.content.pm.PackageManager
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.material3.Button

/*
 * Copyright 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import android.net.Uri
import android.provider.Settings
import androidx.compose.animation.animateContentSize
import androidx.compose.foundation.layout.Box
import androidx.compose.material.icons.rounded.Settings
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.TextButton
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.MultiplePermissionsState
import com.google.accompanist.permissions.isGranted
import com.google.accompanist.permissions.rememberMultiplePermissionsState

/**
 * The PermissionBox uses a [Box] to show a simple permission request UI when the provided [permission]
 * is revoked or the provided [onGranted] content if the permission is granted.
 *
 * This composable follows the permission request flow but for a complete example check the samples
 * under privacy/permissions
 */
@Composable
fun PermissionBox(
    modifier: Modifier = Modifier,
    permission: String,
    description: String? = null,
    contentAlignment: Alignment = Alignment.TopStart,
    onGranted: @Composable BoxScope.() -> Unit,
) {
    PermissionBox(
        modifier,
        permissions = listOf(permission),
        requiredPermissions = listOf(permission),
        description,
        contentAlignment,
    ) { onGranted() }
}

/**
 * A variation of [PermissionBox] that takes a list of permissions and only calls [onGranted] when
 * all the [requiredPermissions] are granted.
 *
 * By default it assumes that all [permissions] are required.
 */
@OptIn(ExperimentalPermissionsApi::class)
@Composable
fun PermissionBox(
    modifier: Modifier = Modifier,
    permissions: List<String>,
    requiredPermissions: List<String> = permissions,
    description: String? = null,
    contentAlignment: Alignment = Alignment.TopStart,
    onGranted: @Composable BoxScope.(List<String>) -> Unit,
) {
    val context = LocalContext.current
    var errorText by remember {
        mutableStateOf("")
    }

    val permissionState = rememberMultiplePermissionsState(permissions = permissions) { map ->
        val rejectedPermissions = map.filterValues { !it }.keys
        errorText = if (rejectedPermissions.none { it in requiredPermissions }) {
            ""
        } else {
            "${rejectedPermissions.joinToString()} required for the sample"
        }
    }
    val allRequiredPermissionsGranted =
        permissionState.revokedPermissions.none { it.permission in requiredPermissions }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .then(modifier),
        contentAlignment = if (allRequiredPermissionsGranted) {
            contentAlignment
        } else {
            Alignment.Center
        },
    ) {
        if (allRequiredPermissionsGranted) {
            onGranted(
                permissionState.permissions
                    .filter { it.status.isGranted }
                    .map { it.permission },
            )
        } else {
            PermissionScreen(
                permissionState,
                description,
                errorText,
            )

            FloatingActionButton(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(16.dp),
                onClick = {
                    val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                        flags = Intent.FLAG_ACTIVITY_NEW_TASK
                        data = Uri.parse("package:${context.packageName}")
                    }
                    context.startActivity(intent)
                },
            ) {
                Icon(imageVector = Icons.Rounded.Settings, contentDescription = "App settings")
            }
        }
    }
}

@OptIn(ExperimentalPermissionsApi::class)
@Composable
private fun PermissionScreen(
    state: MultiplePermissionsState,
    description: String?,
    errorText: String,
) {
    var showRationale by remember(state) {
        mutableStateOf(false)
    }

    val permissions = remember(state.revokedPermissions) {
        state.revokedPermissions.joinToString("\n") {
            " - " + it.permission.removePrefix("android.permission.")
        }
    }
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .animateContentSize(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally,
    ) {
        Text(
            text = "Sample requires permission/s:",
            style = MaterialTheme.typography.titleLarge,
            modifier = Modifier.padding(16.dp),
        )
        Text(
            text = permissions,
            style = MaterialTheme.typography.bodyMedium,
            modifier = Modifier.padding(16.dp),
        )
        if (description != null) {
            Text(
                text = description,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier.padding(16.dp),
            )
        }
        Button(
            onClick = {
                if (state.shouldShowRationale) {
                    showRationale = true
                } else {
                    state.launchMultiplePermissionRequest()
                }
            },
        ) {
            Text(text = "Grant permissions")
        }
        if (errorText.isNotBlank()) {
            Text(
                text = errorText,
                style = MaterialTheme.typography.labelSmall,
                modifier = Modifier.padding(16.dp),
            )
        }
    }
    if (showRationale) {
        AlertDialog(
            onDismissRequest = {
                showRationale = false
            },
            title = {
                Text(text = "Permissions required by the sample")
            },
            text = {
                Text(text = "The sample requires the following permissions to work:\n $permissions")
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        showRationale = false
                        state.launchMultiplePermissionRequest()
                    },
                ) {
                    Text("Continue")
                }
            },
            dismissButton = {
                TextButton(
                    onClick = {
                        showRationale = false
                    },
                ) {
                    Text("Dismiss")
                }
            },
        )
    }
}


/**
 * This composable wraps the permission logic and checks if bluetooth it's available and enabled
 */
@RequiresApi(Build.VERSION_CODES.M)
@Composable
fun BluetoothSampleBox(
    extraPermissions: Set<String> = emptySet(),
    content: @Composable BoxScope.(BluetoothAdapter) -> Unit,
) {
    val context = LocalContext.current
    val packageManager = context.packageManager
    val bluetoothAdapter = context.getSystemService(BluetoothManager::class.java).adapter

    // If we derive physical location from BT devices or if the device runs on Android 11 or below
    // we need location permissions otherwise we don't need to request them (see AndroidManifest).
    val locationPermission = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S) {
        setOf(
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION,
        )
    } else {
        emptySet()
    }

    // For Android 12 and above we only need connect and scan
    val bluetoothPermissionSet = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        setOf(
            Manifest.permission.BLUETOOTH_CONNECT,
            Manifest.permission.BLUETOOTH_SCAN,
        )
    } else {
        setOf(
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN,
        )
    }

    PermissionBox(
        permissions = (bluetoothPermissionSet + locationPermission + extraPermissions).toList(),
        contentAlignment = Alignment.Center,
    ) {
        // Check to see if the Bluetooth classic feature is available.
        val hasBT = packageManager.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH)
        // Check to see if the BLE feature is available.
        val hasBLE = packageManager.hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)
        // Check if the adapter is enabled
        var isBTEnabled by remember {
            mutableStateOf(bluetoothAdapter?.isEnabled == true)
        }

        when {
            bluetoothAdapter == null || !hasBT -> MissingFeatureText(text = "No bluetooth available")
            !hasBLE -> MissingFeatureText(text = "No bluetooth low energy available")
            !isBTEnabled -> BluetoothDisabledScreen { isBTEnabled = true }
            else -> content(bluetoothAdapter)
        }
    }
}

@Composable
fun BluetoothDisabledScreen(onEnabled: () -> Unit) {
    val result =
        rememberLauncherForActivityResult(ActivityResultContracts.StartActivityForResult()) {
            if (it.resultCode == Activity.RESULT_OK) {
                onEnabled()
            }
        }
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Text(text = "Bluetooth is disabled")
        Button(
            onClick = {
                result.launch(Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE))
            },
        ) {
            Text(text = "Enable")
        }
    }
}

@Composable
private fun MissingFeatureText(text: String) {
    Text(
        text = text,
        style = MaterialTheme.typography.headlineSmall,
        color = MaterialTheme.colorScheme.error,
    )
}

@SuppressLint("MissingPermission")
@RequiresApi(Build.VERSION_CODES.M)
@Composable
fun FindBLEDevicesSample() {
    BluetoothSampleBox {
        FindDevicesScreen {
            Log.d("FindDeviceSample", "Name: ${it.name} Address: ${it.address} Type: ${it.type}")
        }
    }
}

@SuppressLint("InlinedApi", "MissingPermission")
@RequiresApi(Build.VERSION_CODES.M)
@RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
@Composable
internal fun FindDevicesScreen(onConnect: (BluetoothDevice) -> Unit) {
    val context = LocalContext.current
    val adapter = checkNotNull(context.getSystemService(BluetoothManager::class.java).adapter)
    var scanning by remember {
        mutableStateOf(true)
    }
    val devices = remember {
        mutableStateListOf<BluetoothDevice>()
    }
    val pairedDevices = remember {
        // Get a list of previously paired devices
        mutableStateListOf<BluetoothDevice>(*adapter.bondedDevices.toTypedArray())
    }
    val sampleServerDevices = remember {
        mutableStateListOf<BluetoothDevice>()
    }
    val scanSettings: ScanSettings = ScanSettings.Builder()
        .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
        .setScanMode(ScanSettings.SCAN_MODE_BALANCED)
        .build()

    // This effect will start scanning for devices when the screen is visible
    // If scanning is stop removing the effect will stop the scanning.
    if (scanning) {
        BluetoothScanEffect(
            scanSettings = scanSettings,
            onScanFailed = {
                scanning = false
                Log.w("FindBLEDevicesSample", "Scan failed with error: $it")
            },
            onDeviceFound = { scanResult ->
                if (!devices.contains(scanResult.device)) {
                    devices.add(scanResult.device)
                }
            },
        )
        // Stop scanning after a while
        LaunchedEffect(true) {
            delay(15000)
            scanning = false
        }
    }

    Column(Modifier.fillMaxSize()) {
        Row(
            Modifier
                .fillMaxWidth()
                .height(54.dp)
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Text(text = "Available devices", style = MaterialTheme.typography.titleSmall)
            if (scanning) {
                CircularProgressIndicator(modifier = Modifier.size(24.dp), strokeWidth = 2.dp)
            } else {
                IconButton(
                    onClick = {
                        devices.clear()
                        scanning = true
                    },
                ) {
                    Icon(imageVector = Icons.Rounded.Refresh, contentDescription = null)
                }
            }
        }

        LazyColumn(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            if (devices.isEmpty()) {
                item {
                    Text(text = "No devices found")
                }
            }
            items(devices) { item ->
                BluetoothDeviceItem(
                    bluetoothDevice = item,
                    isSampleServer = sampleServerDevices.contains(item),
                    onConnect = onConnect,
                )
            }

            if (pairedDevices.isNotEmpty()) {
                item {
                    Text(text = "Saved devices", style = MaterialTheme.typography.titleSmall)
                }
                items(pairedDevices) {
                    BluetoothDeviceItem(
                        bluetoothDevice = it,
                        onConnect = onConnect,
                    )
                }
            }
        }
    }

}

@SuppressLint("MissingPermission")
@Composable
internal fun BluetoothDeviceItem(
    bluetoothDevice: BluetoothDevice,
    isSampleServer: Boolean = false,
    onConnect: (BluetoothDevice) -> Unit,
) {
    Row(
        modifier = Modifier
            .padding(vertical = 8.dp)
            .fillMaxWidth()
            .clickable { onConnect(bluetoothDevice) },
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Text(
            if (isSampleServer) {
                "GATT Sample server"
            } else {
                bluetoothDevice.name ?: "N/A"
            },
            style = if (isSampleServer) {
                TextStyle(fontWeight = FontWeight.Bold)
            } else {
                TextStyle(fontWeight = FontWeight.Normal)
            },
        )
        Text(bluetoothDevice.address)
        val state = when (bluetoothDevice.bondState) {
            BluetoothDevice.BOND_BONDED -> "Paired"
            BluetoothDevice.BOND_BONDING -> "Pairing"
            else -> "None"
        }
        Text(text = state)

    }
}

@SuppressLint("InlinedApi")
@RequiresPermission(Manifest.permission.BLUETOOTH_SCAN)
@Composable
private fun BluetoothScanEffect(
    scanSettings: ScanSettings,
    lifecycleOwner: LifecycleOwner = LocalLifecycleOwner.current,
    onScanFailed: (Int) -> Unit,
    onDeviceFound: (device: ScanResult) -> Unit,
) {
    val context = LocalContext.current
    val adapter = context.getSystemService(BluetoothManager::class.java).adapter

    if (adapter == null) {
        onScanFailed(ScanCallback.SCAN_FAILED_INTERNAL_ERROR)
        return
    }

    val currentOnDeviceFound by rememberUpdatedState(onDeviceFound)

    DisposableEffect(lifecycleOwner, scanSettings) {
        val leScanCallback: ScanCallback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                super.onScanResult(callbackType, result)
                currentOnDeviceFound(result)
            }

            override fun onScanFailed(errorCode: Int) {
                super.onScanFailed(errorCode)
                onScanFailed(errorCode)
            }
        }

        val observer = LifecycleEventObserver { _, event ->
            // Start scanning once the app is in foreground and stop when in background
            if (event == Lifecycle.Event.ON_START) {
                adapter.bluetoothLeScanner.startScan(null, scanSettings, leScanCallback)
            } else if (event == Lifecycle.Event.ON_STOP) {
                adapter.bluetoothLeScanner.stopScan(leScanCallback)
            }
        }

        // Add the observer to the lifecycle
        lifecycleOwner.lifecycle.addObserver(observer)

        // When the effect leaves the Composition, remove the observer and stop scanning
        onDispose {
            lifecycleOwner.lifecycle.removeObserver(observer)
            adapter.bluetoothLeScanner.stopScan(leScanCallback)
        }
    }
}