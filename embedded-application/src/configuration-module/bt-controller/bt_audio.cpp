#include "bt_audio.hpp"

#include <iostream>
#include <thread>
#include <chrono>

GMainLoop* BTAudioController::main_loop = nullptr;
BlueZDevice* BTAudioController::connected_device = nullptr;

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
	GError *error = nullptr;
	bool error_occurred = false;

	GDBusProxy *adapter_proxy = BTAudioController::create_adapter_proxy();
	if (adapter_proxy == nullptr) {
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

int64_t BTAudioController::connect_and_pair_device(BlueZDevice *device) {
	bool error = false;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = BTAudioController::create_device_proxy(device);
	if (device_proxy == nullptr) {
		printf("Failed to create device proxy\n");
		error = true;
		goto cleanup;
	}

	if (!BTAudioController::is_paired(device_proxy)) {
		if (pair_device(device_proxy) < 0) {
			printf("Failed to pair to device\n");
			error = true;
			goto cleanup;
		}
	}

	if (!BTAudioController::is_connected(device_proxy)) {
		if (connect_to_device_profile(device_proxy, A2DP_SINK_UUID) < 0) {
			printf("Failed to connect to profile\n");
			error = true;
			goto cleanup;
		}
	}

	cleanup:
	if (device_proxy) g_object_unref(device_proxy);
	
	if (error) return -1;
	else return 0;
}

BlueZDevice* BTAudioController::find_device(vector<BlueZDevice>& devices, const char *name) {
	for (uint64_t idx = 0; idx < devices.size(); idx++) {
		if (strcmp(devices[idx].name, name) == 0) {
			return &devices[idx];
		}
	}
	return nullptr;
}

bool BTAudioController::is_paired(GDBusProxy *proxy) {
	if (proxy == nullptr) return false;

	GVariant *result = nullptr;
	bool is_paired = false;

	result = BTAudioController::get_proxy_property(proxy, BLUEZ_DEVICE_IFACE, "Paired");
	if (result == nullptr) {
		printf("Failed to get 'Paired' property\n");
		goto cleanup;
	}

	is_paired = BTAudioController::get_boolean_value(result);

	cleanup:
	g_variant_unref(result);
	
	return is_paired;
}

bool BTAudioController::is_connected(GDBusProxy *proxy) {
	if (proxy == nullptr) return false;

	GVariant *result = nullptr;
	bool is_connected = false;

	result = BTAudioController::get_proxy_property(proxy, BLUEZ_DEVICE_IFACE, "Connected");
	if (result == nullptr) {
		printf("Failed to get 'Connected' property\n");
		goto cleanup;
	}

	is_connected = BTAudioController::get_boolean_value(result);

	cleanup:
	g_variant_unref(result);
	
	return is_connected;
}

bool BTAudioController::get_boolean_value(GVariant *variant) {
	if (variant == nullptr) return false;

	GVariant *variant_container = nullptr;
	GVariant *boolean_variant = nullptr;
	bool value = false;

	variant_container = g_variant_get_child_value(variant, 0);
	if (variant_container == nullptr) {
		goto cleanup;
	}

	boolean_variant = g_variant_get_variant(variant_container);
	if (boolean_variant == nullptr) {
		goto cleanup;
	}

	value = g_variant_get_boolean(boolean_variant);

	cleanup:
	if (boolean_variant) g_variant_unref(boolean_variant);
	if (variant_container) g_variant_unref(variant_container);

	return value;
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

		if (BTAudioController::scan_devices(devices, 3) < 0) {
			printf("Failed to scan devices\n");
			BTAudioController::cleanup(devices);
			continue;
		}

		BTAudioController::print_devices(devices);

		BTAudioController::connected_device = BTAudioController::find_device(devices, "QCY H3");
		if (BTAudioController::connected_device == nullptr) {
			BTAudioController::cleanup(devices);
			continue;
		}

		printf("Connecting to device: %s (%s)\n", BTAudioController::connected_device->name, BTAudioController::connected_device->address);

		if (BTAudioController::connect_and_pair_device(BTAudioController::connected_device) < 0) {
			printf("Failed to connect and pair to device\n");
			BTAudioController::cleanup(devices);
			continue;
		}

		printf("BT Audio Controller running...\n");
		if (BTAudioController::main_loop) {
			g_main_loop_run(BTAudioController::main_loop);
		}
	}
}

void BTAudioController::cleanup(vector<BlueZDevice>& devices) {
	devices.clear();

	if (BTAudioController::main_loop) {
		g_main_loop_quit(BTAudioController::main_loop);
		g_main_loop_unref(BTAudioController::main_loop);
		BTAudioController::main_loop = nullptr;
		BTAudioController::connected_device = nullptr;
	}
}
