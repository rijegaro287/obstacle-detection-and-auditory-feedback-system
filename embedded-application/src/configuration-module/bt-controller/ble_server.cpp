#include "ble_server.hpp"

#include <iostream>
#include <thread>
#include <chrono>

GMainLoop* BLEServer::main_loop = nullptr;
GDBusConnection* BLEServer::connection = nullptr;

GDBusNodeInfo* BLEServer::app_info = nullptr;
GDBusNodeInfo* BLEServer::char_info = nullptr;
GDBusNodeInfo* BLEServer::adv_info = nullptr;

void BLEServer::handle_app_method_call(GDBusConnection* connection,
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

void BLEServer::handle_char_method_call(GDBusConnection* connection,
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

GVariant* BLEServer::handle_char_get_property(GDBusConnection *connection,
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

GVariant* BLEServer::handle_adv_get_property(GDBusConnection* connection,
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

int64_t BLEServer::register_application() {
	GError *error = nullptr;

	const GDBusInterfaceVTable app_vtable = {
		BLEServer::handle_app_method_call,
		nullptr,
		nullptr
	};

	// static const GDBusInterfaceVTable service_vtable = {
	// 	nullptr,
	// 	handle_service_get_property,
	// 	nullptr
	// };

	const GDBusInterfaceVTable char_vtable = {
		BLEServer::handle_char_method_call,
		BLEServer::handle_char_get_property,
		nullptr
	};

	const GDBusInterfaceVTable adv_vtable = {
		nullptr,
		handle_adv_get_property,
		nullptr
	};

	g_dbus_connection_register_object(
		BLEServer::connection,
		APP_PATH,
		BLEServer::app_info->interfaces[0],
		&app_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		printf("Error registering app object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_register_object(
		BLEServer::connection,
		CHARACTERISTIC_PATH,
		BLEServer::char_info->interfaces[0],
		&char_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		printf("Error registering char object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	g_dbus_connection_register_object(
		BLEServer::connection,
		ADVERTISING_PATH,
		BLEServer::adv_info->interfaces[0],
		&adv_vtable,
		nullptr,
		nullptr,
		&error
	);

	if (error) {
		printf("Error registering adv object: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	return 0;
}

int64_t BLEServer::advertise_application() {
	GDBusProxy *adapter_proxy = BLEServer::create_adapter_proxy();
	if (adapter_proxy == nullptr) {
		printf("Error creating adapter proxy\n");
		return -1;
	}

	if (BLEServer::set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Powered",
																		g_variant_new_boolean(TRUE))) return -1;

	if (BLEServer::set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Discoverable",
																		g_variant_new_boolean(TRUE))) return -1;

	if (BLEServer::set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Pairable",
																		g_variant_new_boolean(TRUE))) return -1;

	g_dbus_connection_call(
		BLEServer::connection,
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

	return 0;
}

int64_t BLEServer::init() {
	GError *error = nullptr;

  BLEServer::main_loop = g_main_loop_new(nullptr, FALSE);
  if (BLEServer::main_loop == nullptr) {
    printf("Failed to create GMainLoop\n");
		g_error_free(error);
    return -1;
  }

  BLEServer::connection =  BLEServer::create_system_bus_connection();
	if (BLEServer::connection == nullptr) {
		printf("Failed to create D-Bus connection\n");
		g_error_free(error);
		return -1;
	}

  BLEServer::app_info = g_dbus_node_info_new_for_xml(APP_XML, &error);
  if (error) {
    printf("Failed to parse introspection XML: %s\n", error->message);
    g_error_free(error);
		return -1;
  }

  BLEServer::char_info = g_dbus_node_info_new_for_xml(CHAR_XML, &error);
  if (error) {
    printf("Failed to parse characteristic XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  BLEServer::adv_info = g_dbus_node_info_new_for_xml(ADV_XML, &error);
  if (error) {
    printf("Failed to parse advertising XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  return 0;
}

void BLEServer::start_server() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (BLEServer::init()) {
			printf("Failed to initialize BLEServer\n");
			BLEServer::cleanup();
			continue;
		}

		if (BLEServer::register_application() < 0) {
			printf("Failed to register application\n");
			BLEServer::cleanup();
			continue;
		}

		if (BLEServer::advertise_application() < 0) {
			printf("Failed to advertise application\n");
			BLEServer::cleanup();
			continue;
		}

		printf("BLE GATT server running...\n");
		if (BLEServer::main_loop) {
			g_main_loop_run(BLEServer::main_loop);
		}
	}
}

void BLEServer::cleanup() {
	if (BLEServer::app_info) g_dbus_node_info_unref(BLEServer::app_info);
  if (BLEServer::char_info) g_dbus_node_info_unref(BLEServer::char_info);
  if (BLEServer::adv_info) g_dbus_node_info_unref(BLEServer::adv_info);
  if (BLEServer::connection) g_object_unref(BLEServer::connection);
	
  BLEServer::app_info = nullptr;
  BLEServer::char_info = nullptr;
  BLEServer::adv_info = nullptr;
  BLEServer::connection = nullptr;
	
	if (BLEServer::main_loop) {
		g_main_loop_quit(BLEServer::main_loop);
		g_main_loop_unref(BLEServer::main_loop);
		BLEServer::main_loop = nullptr;
	}
}
