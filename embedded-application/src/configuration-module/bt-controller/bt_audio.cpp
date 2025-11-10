/**
 * @file bt_audio.cpp
 * @brief Implements high-level Bluetooth audio management helpers.
 */

#include "bt_audio.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#include "control_iface.hpp"

#ifdef UNIT_TESTING
BTAudioController::TestOverrides BTAudioController::test_overrides{};

void BTAudioController::reset_test_overrides() {
	test_overrides = TestOverrides{};
}
#endif

BTAudioController& BTAudioController::get_instance() {
	static BTAudioController instance;
	return instance;
}

/**
 * @brief Prepare GLib primitives and reset runtime state.
 */
BTAudioController::BTAudioController() {
	GError *error = nullptr;
	this->main_loop = g_main_loop_new(nullptr, FALSE);
  if (this->main_loop == nullptr) {
    printf("Failed to create GMainLoop\n");
		g_error_free(error);
  }
	this->connected_device = nullptr;
}

int64_t BTAudioController::start_discovery() {
#ifdef UNIT_TESTING
	if (test_overrides.start_discovery) {
		return test_overrides.start_discovery();
	}
#endif
	GError *error = nullptr;
	GVariant *result = nullptr;
	GDBusProxy *adapter_proxy = nullptr;

	adapter_proxy = this->create_adapter_proxy();
	if (adapter_proxy == nullptr) {
		printf("Error creating adapter object_manager_proxy: %s\n", error->message);
		goto cleanup;
	}

	result = g_dbus_proxy_call_sync(
		adapter_proxy,
		"StartDiscovery",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		10000,
		nullptr,
		&error
	);
	
	cleanup:
	if (adapter_proxy) g_object_unref(adapter_proxy);
	if (result) g_variant_unref(result);
	if (error) {
		printf("Error starting discovery: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BTAudioController::stop_discovery() {
#ifdef UNIT_TESTING
	if (test_overrides.stop_discovery) {
		return test_overrides.stop_discovery();
	}
#endif
	GError *error = nullptr;
	GVariant *result = nullptr;
	GDBusProxy *adapter_proxy = nullptr;

	adapter_proxy = this->create_adapter_proxy();
	if (adapter_proxy == nullptr) {
		printf("Error creating adapter object_manager_proxy: %s\n", error->message);
		goto cleanup;
	}

	result = g_dbus_proxy_call_sync(
		adapter_proxy,
		"StopDiscovery",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		10000,
		nullptr,
		&error
	);
	
	cleanup:
	if (adapter_proxy) g_object_unref(adapter_proxy);
	if (result) g_variant_unref(result);
	if (error) {
		printf("Error stopping discovery: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BTAudioController::get_discovered_devices(vector<BlueZDevice>& devices) {
#ifdef UNIT_TESTING
	if (test_overrides.get_discovered_devices) {
		return test_overrides.get_discovered_devices(devices);
	}
#endif
	bool error = false;

	GDBusProxy *object_manager_proxy = nullptr;
	GVariant *managed_objects = nullptr;
	GVariant *managed_devices = nullptr;
	
	int64_t device_count = 0;

	object_manager_proxy = this->create_object_manager_proxy();
	if (object_manager_proxy == nullptr) {
		error = true;
		goto cleanup;
	}

	managed_objects = this->get_managed_objects(object_manager_proxy);
	if (managed_objects == nullptr) {
		error = true;
		goto cleanup;
	}

	managed_devices = g_variant_get_child_value(managed_objects, 0);
	if (managed_devices == nullptr) {
		error = true;
		goto cleanup;
	}

	device_count = this->parse_devices(managed_devices, devices);
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

int64_t BTAudioController::find_device_idx(vector<BlueZDevice>& devices, string address) {
	for (uint64_t idx = 0; idx < devices.size(); idx++) {
		if (devices[idx].address == address) {
			return idx;
		}
	}
	return -1;
}

int64_t BTAudioController::pair_device(BlueZDevice& device) {
#ifdef UNIT_TESTING
	if (test_overrides.pair_device) {
		return test_overrides.pair_device(device);
	}
#endif
	bool found_error = false;
	GError* error = nullptr;
	GVariant *result = nullptr;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = this->create_device_proxy(device);
	if (device_proxy == nullptr) {
		goto cleanup;
	}

	result = g_dbus_proxy_call_sync(
		device_proxy,
		"Pair",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		10000,
		nullptr,
		&error
	);

	cleanup:
	if (device_proxy) g_object_unref(device_proxy);
	if (result) g_variant_unref(result);
	if (found_error || error) {
		printf("Error pairing to device: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BTAudioController::connect_device(BlueZDevice& device) {
#ifdef UNIT_TESTING
	if (test_overrides.connect_device) {
		return test_overrides.connect_device(device);
	}
#endif
	bool found_error = false;
	GError* error = nullptr;
	GVariant *result = nullptr;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = this->create_device_proxy(device);
	if (device_proxy == nullptr) {
		goto cleanup;
	}

	result = g_dbus_proxy_call_sync(
		device_proxy,
		"ConnectProfile",
		g_variant_new("(s)", A2DP_SINK_UUID),
		G_DBUS_CALL_FLAGS_NONE,
		10000,
		nullptr,
		&error
	);

	cleanup:
	if (device_proxy) g_object_unref(device_proxy);
	if (result) g_variant_unref(result);
	if (found_error || error) {
		printf("Error connecting to device: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BTAudioController::disconnect_device(BlueZDevice& device) {
#ifdef UNIT_TESTING
	if (test_overrides.disconnect_device) {
		return test_overrides.disconnect_device(device);
	}
#endif
	bool found_error = false;
	GError* error = nullptr;
	GVariant *result = nullptr;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = this->create_device_proxy(device);
	if (device_proxy == nullptr) {
		goto cleanup;
	}

	result = g_dbus_proxy_call_sync(
		device_proxy,
		"Disconnect",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		10000,
		nullptr,
		&error
	);

	cleanup:
	if (device_proxy) g_object_unref(device_proxy);
	if (result) g_variant_unref(result);
	if (found_error || error) {
		printf("Error disconnecting from device: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

bool BTAudioController::is_paired(BlueZDevice& device) {
#ifdef UNIT_TESTING
	if (test_overrides.is_paired) {
		return test_overrides.is_paired(device);
	}
#endif
	bool is_paired = false;
	GVariant *result = nullptr;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = this->create_device_proxy(device);
	if (device_proxy == nullptr) {
		goto cleanup;
	}

	result = this->get_proxy_property(device_proxy, BLUEZ_DEVICE_IFACE, "Paired");
	if (result == nullptr) {
		printf("Failed to get 'Paired' property\n");
		goto cleanup;
	}

	is_paired = this->get_boolean_value(result);

	cleanup:
	g_variant_unref(result);
	g_object_unref(device_proxy);
	
	return is_paired;
}

bool BTAudioController::is_connected(BlueZDevice& device) {
#ifdef UNIT_TESTING
	if (test_overrides.is_connected) {
		return test_overrides.is_connected(device);
	}
#endif
	bool is_connected = false;
	GVariant *result = nullptr;
	GDBusProxy *device_proxy = nullptr;

	device_proxy = this->create_device_proxy(device);
	if (device_proxy == nullptr) {
		goto cleanup;
	}

	result = this->get_proxy_property(device_proxy, BLUEZ_DEVICE_IFACE, "Connected");
	if (result == nullptr) {
		printf("Failed to get 'Connected' property\n");
		goto cleanup;
	}

	is_connected = this->get_boolean_value(result);

	cleanup:
	g_variant_unref(result);
	g_object_unref(device_proxy);
	
	return is_connected;
}

bool BTAudioController::get_boolean_value(GVariant *variant) {
	if (variant == nullptr) {
		return false;
	}

	if (g_variant_is_of_type(variant, G_VARIANT_TYPE_BOOLEAN)) {
		return g_variant_get_boolean(variant);
	}

	if (g_variant_is_of_type(variant, G_VARIANT_TYPE_VARIANT)) {
		GVariant *inner = g_variant_get_variant(variant);
		bool value = BTAudioController::get_boolean_value(inner);
		if (inner) {
			g_variant_unref(inner);
		}
		return value;
	}

	if (g_variant_is_of_type(variant, G_VARIANT_TYPE_TUPLE) && g_variant_n_children(variant) > 0) {
		GVariant *child = g_variant_get_child_value(variant, 0);
		bool value = BTAudioController::get_boolean_value(child);
		if (child) {
			g_variant_unref(child);
		}
		return value;
	}

	return false;
}


void BTAudioController::cleanup(vector<BlueZDevice>& devices) {
#ifdef UNIT_TESTING
	if (test_overrides.cleanup) {
		test_overrides.cleanup(devices);
	}
#endif
	if (this->main_loop) {
		g_main_loop_quit(this->main_loop);
		g_main_loop_unref(this->main_loop);
		this->main_loop = nullptr;
	}

	devices.clear();
	this->connected_device = nullptr;
}
