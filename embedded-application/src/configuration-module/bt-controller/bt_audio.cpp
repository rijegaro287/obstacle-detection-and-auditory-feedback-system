#include "bt_audio.hpp"

#include <iostream>
#include <thread>
#include <chrono>

GMainLoop* BTAudioController::main_loop = nullptr;

int64_t BTAudioController::init() {
	GError *error = nullptr;
	
	BTAudioController::main_loop = g_main_loop_new(nullptr, FALSE);
  if (BTAudioController::main_loop == nullptr) {
    printf("Failed to create GMainLoop\n");
		g_error_free(error);
    return -1;
  }

	return 0;
}

int64_t BTAudioController::start_discovery(GDBusProxy *proxy) {
	GError *error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"StartDiscovery",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error starting discovery: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	if (result) g_variant_unref(result);

	return 0;
}

int64_t BTAudioController::stop_discovery(GDBusProxy *proxy) {
	GError *error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
			proxy,
			"StopDiscovery",
			nullptr,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			nullptr,
			&error
		);
	
	if (error) {
		printf("Error stopping discovery: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	if (result) g_variant_unref(result);

	return 0;
}

int64_t BTAudioController::get_discovered_devices(vector<BlueZDevice>& devices) {
	bool error = false;

	GDBusProxy *object_manager_proxy = nullptr;
	GVariant *managed_objects = nullptr;
	GVariant *managed_devices = nullptr;
	
	int64_t device_count = 0;

	object_manager_proxy = BTAudioController::create_object_manager_proxy();
	if (object_manager_proxy == nullptr) {
		error = true;
		goto cleanup;
	}

	managed_objects = BTAudioController::get_managed_objects(object_manager_proxy);
	if (managed_objects == nullptr) {
		error = true;
		goto cleanup;
	}

	managed_devices = g_variant_get_child_value(managed_objects, 0);
	if (managed_devices == nullptr) {
		error = true;
		goto cleanup;
	}

	device_count = BTAudioController::parse_devices(managed_devices, devices);
	if (device_count < 0) {
		error = true;
		goto cleanup;
	}

	cleanup:
	if (object_manager_proxy) g_object_unref(object_manager_proxy);
	if (managed_objects) g_variant_unref(managed_objects);
	if (managed_devices) g_variant_unref(managed_devices);
	if (error) return -1;

	return device_count;
}

int64_t BTAudioController::scan_devices(vector<BlueZDevice>& devices, uint64_t timeout_sec) {
	GError *error = NULL;
	bool error_occurred = false;

	GDBusProxy *adapter_proxy = BTAudioController::create_adapter_proxy();
	if (adapter_proxy == NULL) {
		printf("Error creating adapter object_manager_proxy: %s\n", error->message);
		error_occurred = true;
		goto cleanup;
	}

	if (BTAudioController::start_discovery(adapter_proxy) < 0) {
		printf("Failed to start discovery\n");
		error_occurred = true;
		goto cleanup;
	}

	this_thread::sleep_for(std::chrono::seconds(timeout_sec));

	if (BTAudioController::stop_discovery(adapter_proxy) < 0) {
		printf("Failed to stop discovery\n");
		error_occurred = true;
		goto cleanup;
	}

	if (BTAudioController::get_discovered_devices(devices) < 0) {
		printf("Failed to scan devices\n");
		error_occurred = true;
		goto cleanup;
	}

	cleanup:
	if (adapter_proxy) g_object_unref(adapter_proxy);
	if (error_occurred) return -1;
	
	return 0;
}

int64_t BTAudioController::pair_device(GDBusProxy *proxy) {
	if (proxy == nullptr) return -1;
	GError* error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"Pair",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error pairing to device: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	if (result) g_variant_unref(result);

	return 0;
}

int64_t BTAudioController::connect_to_device(GDBusProxy *proxy) {
	if (proxy == nullptr) return -1;
	GError* error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"Connect",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error connecting to device: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	if (result) g_variant_unref(result);

	return 0;
}

int64_t BTAudioController::connect_to_device_profile(GDBusProxy *proxy, const char *uuid) {
	if (proxy == nullptr) return -1;
	GError* error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"ConnectProfile",
		g_variant_new("(s)", uuid),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error connecting to profile: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	if (result) g_variant_unref(result);

	return 0;
}

void BTAudioController::start() {
	vector<BlueZDevice> devices;
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (BTAudioController::init() < 0) {
			printf("Failed to initialize Bluetooth Audio Controller\n");
			BTAudioController::cleanup(devices);
			continue;
		}

		if (BTAudioController::scan_devices(devices, 5) < 0) {
			printf("Failed to scan devices\n");
			BTAudioController::cleanup(devices);
			continue;
		}

		BTAudioController::print_devices(devices);

		printf("BT Audio Controller running...\n");
		g_main_loop_run(BTAudioController::main_loop);

	// GDBusProxy *device_proxy = NULL;
	// for (uint64_t idx = 0; idx < device_count; idx++) {
	// 	BlueZDevice device = devices[idx];
	// 	if (strcmp(device.name, "QCY H3") == 0) {
	// 		printf("Connecting to device: %s (%s)\n", device.name, device.address);

	// 		device_proxy = create_device_proxy(device);
	// 		if (device_proxy == NULL) {
	// 			printf("Failed to create device proxy\n");
	// 			status = -1;
	// 			break;
	// 		}

	// 		if (!is_paired(device_proxy)) {
	// 			printf("Device not paired, pairing...\n");
	// 			if (pair_device(device_proxy) < 0) {
	// 				printf("Failed to pair to device\n");
	// 				status = -1;
	// 				break;
	// 			}
	// 		}
	// 		printf("Device paired, connecting...\n");
      
	// 		if (!is_connected(device_proxy)) {
	// 			printf("Device not connected, connecting...\n");
	// 			if (connect_to_device_profile(device_proxy, A2DP_SINK_UUID) < 0) {
	// 				printf("Failed to connect to profile\n");
	// 				status = -1;
	// 				break;
	// 			}
	// 		}
	// 		printf("Device connected successfully!\n");
	// 	}
	// }

	// if (device_proxy) g_object_unref(device_proxy);
	// clear_devices(devices);
	// printf("=======================================\n");
	// this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void BTAudioController::cleanup(vector<BlueZDevice>& devices) {
	devices.clear();

	if (BTAudioController::main_loop) {
		g_main_loop_quit(BTAudioController::main_loop);
		g_main_loop_unref(BTAudioController::main_loop);
		BTAudioController::main_loop = nullptr;
	}
}