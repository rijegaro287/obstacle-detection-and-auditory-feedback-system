#include "bluetooth_controller.hpp"

#include <iostream>
#include <thread>
#include <chrono>

GMainLoop* BLEController::main_loop = nullptr;
GDBusConnection* BLEController::connection = nullptr;

GDBusNodeInfo* BLEController::app_info = nullptr;
GDBusNodeInfo* BLEController::char_info = nullptr;
GDBusNodeInfo* BLEController::adv_info = nullptr;

void BLEController::handle_app_method_call(GDBusConnection* connection,
																					 const gchar* sender,
																					 const gchar* object_path,
																					 const gchar* interface_name,
																					 const gchar* method_name,
																					 GVariant* parameters,
																					 GDBusMethodInvocation* invocation,
																					 gpointer user_data) {
	if (g_strcmp0(method_name, "GetManagedObjects") == 0) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));
		
		// Add service object
		g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
		g_variant_builder_add(&builder, "o", SERVICE_PATH);
		
		// Service interfaces
		g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
		g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
		g_variant_builder_add(&builder, "s", GATT_SERVICE_IFACE);
		
		// Service properties
		g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
		g_variant_builder_add(&builder, "{sv}", "UUID", g_variant_new_string(SERVICE_UUID));
		g_variant_builder_add(&builder, "{sv}", "Primary", g_variant_new_boolean(TRUE));
		g_variant_builder_close(&builder); // Close service properties
		g_variant_builder_close(&builder); // Close service interface
		g_variant_builder_close(&builder); // Close service interfaces
		g_variant_builder_close(&builder); // Close service object
		
		// Add characteristic object
		g_variant_builder_open(&builder, G_VARIANT_TYPE("{oa{sa{sv}}}"));
		g_variant_builder_add(&builder, "o", CHARACTERISTIC_PATH);
		
		// Characteristic interfaces
		g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sa{sv}}"));
		g_variant_builder_open(&builder, G_VARIANT_TYPE("{sa{sv}}"));
		g_variant_builder_add(&builder, "s", GATT_CHARACTERISTIC_IFACE);
		
		// Characteristic properties
		g_variant_builder_open(&builder, G_VARIANT_TYPE("a{sv}"));
		g_variant_builder_add(&builder, "{sv}", "UUID", g_variant_new_string(CHARACTERISTIC_UUID));
		g_variant_builder_add(&builder, "{sv}", "Service", g_variant_new_object_path(SERVICE_PATH));
		
		// Flags array
		GVariantBuilder flags_builder;
		g_variant_builder_init(&flags_builder, G_VARIANT_TYPE("as"));
		g_variant_builder_add(&flags_builder, "s", "read");
		g_variant_builder_add(&flags_builder, "s", "write");
		g_variant_builder_add(&builder, "{sv}", "Flags", g_variant_builder_end(&flags_builder));
		
		g_variant_builder_close(&builder); // Close characteristic properties
		g_variant_builder_close(&builder); // Close characteristic interface
		g_variant_builder_close(&builder); // Close characteristic interfaces
		g_variant_builder_close(&builder); // Close characteristic object
		
		GVariant *result = g_variant_builder_end(&builder);
		g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&result, 1));
	}
}

void BLEController::handle_char_method_call(GDBusConnection* connection,
																						const gchar* sender,
																						const gchar* object_path,
																						const gchar* interface_name,
																						const gchar* method_name,
																						GVariant* parameters,
																						GDBusMethodInvocation* invocation,
																						gpointer user_data) {
	if (g_strcmp0(method_name, "ReadValue") == 0) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("ay"));
		g_variant_builder_add(&builder, "y", 0x48); // 'H'
		g_variant_builder_add(&builder, "y", 0x65); // 'e'
		g_variant_builder_add(&builder, "y", 0x6c); // 'l'
		g_variant_builder_add(&builder, "y", 0x6c); // 'l'
		g_variant_builder_add(&builder, "y", 0x6f); // 'o'
		
		GVariant* result = g_variant_new("(ay)", &builder);
		g_dbus_method_invocation_return_value(invocation, result);
	}
	else if (g_strcmp0(method_name, "WriteValue") == 0) {
		printf("Write value received\n");
		g_dbus_method_invocation_return_value(invocation, nullptr);
	}
}

GVariant* BLEController::handle_char_get_property(GDBusConnection *connection,
																									const gchar *sender,
																									const gchar *object_path,
																									const gchar *interface_name,
																									const gchar *property_name,
																									GError** error,
																									gpointer user_data) {
	if (g_strcmp0(property_name, "UUID") == 0) {
		return g_variant_new_string(CHARACTERISTIC_UUID);
	}
	else if (g_strcmp0(property_name, "Service") == 0) {
		return g_variant_new_object_path(SERVICE_PATH);
	}
	else if (g_strcmp0(property_name, "Flags") == 0) {
		const char* flags[] = {"read", "write"};
		return g_variant_new_strv(flags, 2);
	}
	else {
		g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Unknown property: %s", property_name);
	}
	return nullptr;
}

GVariant* BLEController::handle_adv_get_property(GDBusConnection* connection,
																								 const gchar* sender,
																								 const gchar* object_path,
																								 const gchar* interface_name,
																								 const gchar* property_name,
																								 GError** error,
																								 gpointer user_data) {
	if (g_strcmp0(property_name, "Type") == 0) {
		return g_variant_new_string("peripheral");
	}
	else if (g_strcmp0(property_name, "ServiceUUIDs") == 0) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
		g_variant_builder_add(&builder, "s", SERVICE_UUID);
		return g_variant_builder_end(&builder);
	}
	else if (g_strcmp0(property_name, "LocalName") == 0) {
		return g_variant_new_string("GATTServer");
	} 
	else if (g_strcmp0(property_name, "Includes") == 0) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
		g_variant_builder_add(&builder, "s", "local-name");
		return g_variant_builder_end(&builder);
	}

	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Unknown property: %s", property_name);
	return nullptr;
}

int64_t BLEController::register_application() {
	GError *error = nullptr;

	const GDBusInterfaceVTable app_vtable = {
		BLEController::handle_app_method_call,
		nullptr,
		nullptr
	};

	// static const GDBusInterfaceVTable service_vtable = {
	// 	nullptr,
	// 	handle_service_get_property,
	// 	nullptr
	// };

	const GDBusInterfaceVTable char_vtable = {
		BLEController::handle_char_method_call,
		BLEController::handle_char_get_property,
		nullptr
	};

	const GDBusInterfaceVTable adv_vtable = {
		nullptr,
		handle_adv_get_property,
		nullptr
	};

	g_dbus_connection_register_object(
		BLEController::connection,
		APP_PATH,
		BLEController::app_info->interfaces[0],
		&app_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		g_printerr("Error registering app object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_register_object(
		BLEController::connection,
		CHARACTERISTIC_PATH,
		BLEController::char_info->interfaces[0],
		&char_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		g_printerr("Error registering char object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_register_object(
		BLEController::connection,
		ADVERTISING_PATH,
		BLEController::adv_info->interfaces[0],
		&adv_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		g_printerr("Error registering adv object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BLEController::advertise_application() {
	GError *error = nullptr;
	
	g_dbus_connection_call(
		BLEController::connection,
		BLUEZ_SERVICE,
		BLUEZ_ADAPTER_PATH,
		GATT_MANAGER_IFACE,
		"RegisterApplication",
		g_variant_new("(oa{sv})", APP_PATH, nullptr),
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		nullptr,
		nullptr
	);

	if (error) {
		g_printerr("Error registering application: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_call(
		connection,
		BLUEZ_SERVICE,
		BLUEZ_ADAPTER_PATH,
		LE_ADVERTISING_MANAGER_IFACE,
		"RegisterAdvertisement",
		g_variant_new("(oa{sv})", APP_PATH, nullptr),
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		nullptr,
		nullptr
	);

	if (error) {
		g_printerr("Error registering advertisement: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BLEController::init() {
	GError *error = nullptr;

  BLEController::main_loop = g_main_loop_new(nullptr, FALSE);
  if (BLEController::main_loop == nullptr) {
    printf("Failed to create GMainLoop\n");
		g_error_free(error);
    return -1;
  }

  BLEController::connection = create_system_bus_connection();
	if (BLEController::connection == nullptr) {
		printf("Failed to create D-Bus connection\n");
		g_error_free(error);
		return -1;
	}

  BLEController::app_info = g_dbus_node_info_new_for_xml(APP_XML, &error);
  if (error) {
    printf("Failed to parse introspection XML: %s\n", error->message);
    g_error_free(error);
		return -1;
  }

  BLEController::char_info = g_dbus_node_info_new_for_xml(CHAR_XML, &error);
  if (error) {
    printf("Failed to parse characteristic XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  BLEController::adv_info = g_dbus_node_info_new_for_xml(ADV_XML, &error);
  if (error) {
    printf("Failed to parse advertising XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  return 0;
}

void BLEController::start_server() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (BLEController::init()) {
			printf("Failed to initialize BLEController\n");
			BLEController::cleanup();
			continue;
		}

		if (BLEController::register_application() < 0) {
			printf("Failed to register application\n");
			BLEController::cleanup();
			continue;
		}

		if (BLEController::advertise_application() < 0) {
			printf("Failed to advertise application\n");
			BLEController::cleanup();
			continue;
		}

		g_main_loop_run(BLEController::main_loop);
	}
}

void BLEController::cleanup() {
	if (BLEController::app_info) g_dbus_node_info_unref(BLEController::app_info);
  if (BLEController::char_info) g_dbus_node_info_unref(BLEController::char_info);
  if (BLEController::adv_info) g_dbus_node_info_unref(BLEController::adv_info);
  if (BLEController::connection) g_object_unref(BLEController::connection);
	
  BLEController::app_info = nullptr;
  BLEController::char_info = nullptr;
  BLEController::adv_info = nullptr;
  BLEController::connection = nullptr;
	
	if (BLEController::main_loop) {
		g_main_loop_quit(BLEController::main_loop);
		g_main_loop_unref(BLEController::main_loop);
		BLEController::main_loop = nullptr;
	}
}










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

GDBusProxy* create_gatt_manager_proxy() {
	GError* error = nullptr;

	GDBusProxy *gatt_manager_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		nullptr,
		BLUEZ_SERVICE,
		BLUEZ_ADAPTER_PATH,
		GATT_MANAGER_IFACE,
		nullptr,
		&error
	);

	if (error) {
		printf("Error creating gatt_manager_proxy: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return gatt_manager_proxy;
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

// bool is_paired(GDBusProxy *proxy, const char *property) {
// 	if (proxy == nullptr) return false;

// 	GVariant *result = get_proxy_property(proxy, property, "Paired");
// 	if (result == nullptr) return false;

// 	GVariant *value = nullptr;
// 	g_variant_get(result, "(v)", &value);
//  	gboolean paired = g_variant_get_boolean(value);

// 	g_variant_unref(value);
// 	g_variant_unref(result);

// 	return paired;
// }

// bool is_connected(GDBusProxy *proxy) { 
// 	if (proxy == nullptr) return false;

// 	GVariant *result = get_proxy_property(proxy, "Connected");
// 	if (result == nullptr) return false;

// 	GVariant *value = nullptr;
// 	g_variant_get(result, "(v)", &value);
//  	gboolean connected = g_variant_get_boolean(value);

// 	g_variant_unref(value);
// 	g_variant_unref(result);

// 	return connected;
// }

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
