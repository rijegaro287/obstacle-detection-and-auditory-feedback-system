#include "bt_audio.hpp"

#include <iostream>

void print_devices(bluez_device_t *devices, uint64_t device_count) {
	for (size_t i = 0; i < device_count; i++) {
		bluez_device_t device = devices[i];
		printf("- Found device: %s (%s)\n", device.name, device.address);
	}
}

void clear_devices(bluez_device_t *devices) {
	if (devices == nullptr) return;
	for (int i = 0; i < MAX_DEVICES; i++) {
		memset(&devices[i], 0, sizeof(bluez_device_t));
	}
}

void addr_to_path(char *addr, char *dest, uint64_t dest_size) {
	for (uint64_t idx = 0; idx < dest_size - 1 && addr[idx] != '\0'; idx++) {
		char src_char = addr[idx];
		if (src_char == ':') src_char = '_';
		dest[idx] = src_char;
	}
	dest[dest_size - 1] = '\0';
}

GDBusConnection* create_system_bus_connection() {
	GError *error = nullptr;

	GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);

	if (!connection) {
		printf("Error getting system bus connection: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	if (error) {
		printf("Error getting system bus connection: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return connection;
}

GDBusProxy* create_object_manager_proxy() {
	GError* error = nullptr;

	GDBusProxy *object_manager_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		nullptr,
		BLUEZ_SERVICE,
		"/",
		"org.freedesktop.DBus.ObjectManager",
		nullptr,
		&error
	);

	if (error) {
		printf("Error creating object_manager_proxy: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return object_manager_proxy;
}

GDBusProxy* create_adapter_proxy() {
	GError* error = nullptr;

	GDBusProxy *adapter_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		nullptr,
		BLUEZ_SERVICE,
		BLUEZ_ADAPTER_PATH,
		BLUEZ_ADAPTER_IFACE,
		nullptr,
		&error
	);

	if (error) {
		printf("Error creating device proxy: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return adapter_proxy;
}

GDBusProxy* create_device_proxy(bluez_device_t device) {
	GError* error = nullptr;

	char device_path[BLUEZ_DEVICE_BUFFER_L] = {0};
	char device_addr[BLUEZ_DEVICE_BUFFER_S] = {0};

	addr_to_path(device.address, device_addr, BLUEZ_DEVICE_BUFFER_S);

	snprintf(device_path, BLUEZ_DEVICE_BUFFER_L, "%s/dev_%s", BLUEZ_ADAPTER_PATH, device_addr);
	device_path[BLUEZ_DEVICE_BUFFER_L - 1] = '\0';

	GDBusProxy *device_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		nullptr,
		BLUEZ_SERVICE,
		device_path,
		BLUEZ_DEVICE_IFACE,
		nullptr,
		&error
	);

	if (error) {
		printf("Error creating device proxy: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return device_proxy;
}

GVariant* get_managed_objects(GDBusProxy *proxy) {
	GError* error = nullptr;

	GVariant* result = g_dbus_proxy_call_sync(
		proxy,
		"GetManagedObjects",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error getting managed objects: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return result;
}

GVariant* get_proxy_property(GDBusProxy *proxy,
														 const char *interface,
														 const char *property) {
	if (proxy == nullptr || interface == nullptr || property == nullptr)
		return nullptr;

	GError* error = nullptr;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"org.freedesktop.DBus.Properties.Get",
		g_variant_new("(ss)", interface, property),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error getting device property %s: %s\n", property, error->message);
		g_error_free(error);
		return nullptr;
	}

	return result;
}

int64_t set_proxy_property(GDBusProxy *proxy,
													 const char *interface,
													 const char *property,
													 GVariant *value) {
	if (proxy == nullptr || interface == nullptr || property == nullptr || value == nullptr)
		return -1;

	GError *error = nullptr;

	g_dbus_proxy_call_sync(
		proxy,
		"org.freedesktop.DBus.Properties.Set",
		g_variant_new("(ssv)", interface, property, value),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error
	);

	if (error) {
		printf("Error setting device property %s: %s\n", property, error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t start_discovery(GDBusProxy *proxy) {
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

int64_t stop_discovery(GDBusProxy *proxy) {
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

int64_t parse_devices(GVariant *devices_variant, bluez_device_t *dest, uint64_t max_devices) {
	int64_t device_count = 0;
	if (strcmp(g_variant_get_type_string(devices_variant), "a{oa{sa{sv}}}") != 0) return -1;

	GVariantIter devices_iter;
	const gchar *device_name;
	GVariant *ifaces_dict;

	g_variant_iter_init(&devices_iter, devices_variant);
	while (g_variant_iter_next(&devices_iter, "{&o@a{sa{sv}}}", &device_name, &ifaces_dict)) {
		if (ifaces_dict == nullptr) return -1;
		if (strcmp(g_variant_get_type_string(ifaces_dict), "a{sa{sv}}") != 0) continue;

		GVariantIter iface_iter;
		const gchar *iface_name;
		GVariant *iface_dict;

		g_variant_iter_init(&iface_iter, ifaces_dict);
		uint64_t device_idx = 0;
		while (g_variant_iter_next(&iface_iter, "{&s@a{sv}}", &iface_name, &iface_dict)) {
			if (strcmp(iface_name, BLUEZ_DEVICE_IFACE) != 0) continue;
			if (device_count + device_idx >= max_devices) break;

			GVariant *name_variant = g_variant_lookup_value(iface_dict, "Name", nullptr);
			GVariant *addr_variant = g_variant_lookup_value(iface_dict, "Address", nullptr);
			if (!name_variant || !addr_variant) continue;

			const char *name = g_variant_get_string(name_variant, nullptr);
			const char *address = g_variant_get_string(addr_variant, nullptr);

			strncpy(dest[device_count + device_idx].name, name, BLUEZ_DEVICE_BUFFER_L - 1);
			dest[device_count + device_idx].name[BLUEZ_DEVICE_BUFFER_L - 1] = '\0';

			strncpy(dest[device_count + device_idx].address, address, BLUEZ_DEVICE_BUFFER_S - 1);
			dest[device_count + device_idx].address[BLUEZ_DEVICE_BUFFER_S - 1] = '\0';

			device_idx++;

			if (addr_variant) g_variant_unref(addr_variant);
			if (name_variant) g_variant_unref(name_variant);
		}
		device_count += device_idx;
		g_variant_unref(ifaces_dict);
	}
	return device_count;
}

int64_t scan_devices(bluez_device_t *dest, uint64_t max_devices) {
	GError* error = nullptr;

	GDBusProxy *object_manager_proxy = create_object_manager_proxy();
	if (object_manager_proxy == nullptr) return -1;

	GVariant *managed_objects = get_managed_objects(object_manager_proxy);
	if (managed_objects == nullptr) {
		g_object_unref(object_manager_proxy);
		return -1;
	};

	GVariant *managed_devices = g_variant_get_child_value(managed_objects, 0);
	if (managed_devices == nullptr) {
		g_variant_unref(managed_objects);
		g_object_unref(object_manager_proxy);
		return -1;
	}

	int64_t device_count = parse_devices(managed_devices, dest, max_devices);
	if (device_count < 0) {
		g_variant_unref(managed_devices);
		g_variant_unref(managed_objects);
		g_object_unref(object_manager_proxy);
		return -1;
	}

	g_variant_unref(managed_devices);
	g_variant_unref(managed_objects);
	g_object_unref(object_manager_proxy);

	return device_count;
}

int64_t pair_device(GDBusProxy *proxy) {
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

int64_t connect_to_device(GDBusProxy *proxy) {
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

int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid) {
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

// GError *error = NULL;

// GMainLoop *loop = g_main_loop_new(NULL, FALSE);
// GDBusProxy *adapter_proxy = g_dbus_proxy_new_for_bus_sync(
// 	G_BUS_TYPE_SYSTEM,
// 	G_DBUS_PROXY_FLAGS_NONE,
// 	NULL,
// 	BLUEZ_SERVICE,
// 	ADAPTER_PATH,
// 	BLUEZ_ADAPTER_IFACE,
// 	NULL,
// 	&error
// );

// if (error) {
// 	printf("Error creating adapter object_manager_proxy: %s\n", error->message);
// 	g_error_free(error);
// 	return -1;
// }

// bluez_device_t devices[MAX_DEVICES] = {0};
// int64_t status = 0;
// while (true) {
// 	printf("=======================================\n");
// 	printf("Starting discovery...\n");
// 	if (start_discovery(adapter_proxy) < 0) {
// 		printf("Failed to start discovery\n");
// 		status = -1;
// 		break;
// 	}

// 	this_thread::sleep_for(std::chrono::seconds(3));

// 	printf("Stopping discovery...\n");
// 	if (stop_discovery(adapter_proxy) < 0) {
// 		printf("Failed to stop discovery\n");
// 		status = -1;
// 		break;
// 	}

// 	int64_t device_count = scan_devices(devices, MAX_DEVICES);
// 	if (device_count < 0) {
// 		printf("Failed to scan devices\n");
// 		status = -1;
// 		break;
// 	}

// 	print_devices(devices, device_count);

// 	GDBusProxy *device_proxy = NULL;
// 	for (uint64_t idx = 0; idx < device_count; idx++) {
// 		bluez_device_t device = devices[idx];
// 		if (strcmp(device.name, "QCY H3") == 0) {
// 			printf("Connecting to device: %s (%s)\n", device.name, device.address);

// 			device_proxy = create_device_proxy(device);
// 			if (device_proxy == NULL) {
// 				printf("Failed to create device proxy\n");
// 				status = -1;
// 				break;
// 			}

// 			if (!is_paired(device_proxy)) {
// 				printf("Device not paired, pairing...\n");
// 				if (pair_device(device_proxy) < 0) {
// 					printf("Failed to pair to device\n");
// 					status = -1;
// 					break;
// 				}
// 			}
// 			printf("Device paired, connecting...\n");
      
// 			if (!is_connected(device_proxy)) {
// 				printf("Device not connected, connecting...\n");
// 				if (connect_to_device_profile(device_proxy, A2DP_SINK_UUID) < 0) {
// 					printf("Failed to connect to profile\n");
// 					status = -1;
// 					break;
// 				}
// 			}
// 			printf("Device connected successfully!\n");
// 		}
// 	}

// 	if (device_proxy) g_object_unref(device_proxy);
// 	clear_devices(devices);
// 	printf("=======================================\n");
// 	this_thread::sleep_for(std::chrono::seconds(1));
// }

// g_main_loop_unref(loop);
// g_object_unref(adapter_proxy);

// return status;