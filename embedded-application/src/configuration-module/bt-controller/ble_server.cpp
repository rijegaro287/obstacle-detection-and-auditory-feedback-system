#include "ble_server.hpp"

#include "configuration_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

BLEServer& BLEServer::get_instance() {
	static BLEServer instance;
	return instance;
}

BLEServer::BLEServer() {
	this->main_loop = nullptr;
	this->connection = nullptr;

	this->adv_info = nullptr;
	this->app_info = nullptr;
	this->service_info = nullptr;
	this->char_info = nullptr;
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
	else if (g_strcmp0(property_name, "LocalName") == 0) {
		return g_variant_new_string(DEVICE_NAME);
	}
	else if (g_strcmp0(property_name, "Appearance") == 0) {
		return g_variant_new_uint16(HID_APPEARANCE_CODE);
	}
	else if (g_strcmp0(property_name, "Discoverable") == 0) {
		return g_variant_new_boolean(TRUE);
	}
	else if (g_strcmp0(property_name, "DiscoverableTimeout") == 0) {
		return g_variant_new_uint16(0);
	}
	else if (g_strcmp0(property_name, "ScanResponseServiceUUIDs") == 0 ||
					 g_strcmp0(property_name, "ServiceUUIDs") == 0) {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("as"));
		g_variant_builder_add(&builder, "s", SERVICE_UUID);
		return g_variant_builder_end(&builder);
	}

	g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Unknown property: %s", property_name);
	return nullptr;
}


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

GVariant* BLEServer::handle_service_get_property(GDBusConnection *connection,
																								 const gchar *sender,
																								 const gchar *object_path,
																								 const gchar *interface_name,
																								 const gchar *property_name,
																								 GError** error,
																								 gpointer user_data) {
	if (g_strcmp0(property_name, "UUID") == 0) {
		return g_variant_new_string(SERVICE_UUID);
	}
	else if (g_strcmp0(property_name, "Primary") == 0) {
		return g_variant_new_boolean(TRUE);
	}
	else {
		g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED, "Unknown property: %s", property_name);
	}
	return nullptr;
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
		for (char c : IConfiguration::get_response_buffer()) {
			g_variant_builder_add(&builder, "y", (guint8)c);
		}

		GVariant* result = g_variant_new("(ay)", &builder);
		g_dbus_method_invocation_return_value(invocation, result);
	}
	else if (g_strcmp0(method_name, "WriteValue") == 0) {
    GVariant* value_variant = nullptr;
    GVariant* options_variant = nullptr;

    g_variant_get(parameters, "(@ay@a{sv})", &value_variant, &options_variant);
		
		if (value_variant == nullptr) { 
			printf("Failed to get value variant\n");	
		}

		if (options_variant == nullptr) {
			printf("Failed to get options variant\n");
		}

		gsize n_elements;
		const guint8 *data = (guint8*)g_variant_get_fixed_array(value_variant, &n_elements, sizeof(guint8));

		IConfiguration::process_command(string((const char*)data, n_elements));

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

int64_t BLEServer::init() {
	GError *error = nullptr;

  this->main_loop = g_main_loop_new(nullptr, FALSE);
  if (this->main_loop == nullptr) {
    printf("Failed to create GMainLoop\n");
		g_error_free(error);
    return -1;
  }

  this->connection =  this->create_system_bus_connection();
	if (this->connection == nullptr) {
		printf("Failed to create D-Bus connection\n");
		g_error_free(error);
		return -1;
	}

	this->adv_info = g_dbus_node_info_new_for_xml(ADV_XML, &error);
  if (error) {
    printf("Failed to parse advertising XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  this->app_info = g_dbus_node_info_new_for_xml(APP_XML, &error);
  if (error) {
    printf("Failed to parse introspection XML: %s\n", error->message);
    g_error_free(error);
		return -1;
  }

  this->service_info = g_dbus_node_info_new_for_xml(SERVICE_XML, &error);
  if (error) {
    printf("Failed to parse service XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  this->char_info = g_dbus_node_info_new_for_xml(CHAR_XML, &error);
  if (error) {
    printf("Failed to parse characteristic XML: %s\n", error->message);
    g_error_free(error);
    return -1;
  }

  return 0;
}

int64_t BLEServer::register_application() {
	GError *error = nullptr;

	const GDBusInterfaceVTable adv_vtable = {
		nullptr,
		BLEServer::handle_adv_get_property,
		nullptr
	};

	const GDBusInterfaceVTable app_vtable = {
		BLEServer::handle_app_method_call,
		nullptr,
		nullptr
	};

	const GDBusInterfaceVTable service_vtable = {
		nullptr,
		BLEServer::handle_service_get_property,
		nullptr
	};

	const GDBusInterfaceVTable char_vtable = {
		BLEServer::handle_char_method_call,
		BLEServer::handle_char_get_property,
		nullptr
	};

	g_dbus_connection_register_object(
		this->connection,
		APP_PATH,
		this->app_info->interfaces[0],
		&app_vtable,
		nullptr,
		nullptr,
		&error
	);

	g_dbus_connection_register_object(
		this->connection,
		ADVERTISING_PATH,
		this->adv_info->interfaces[0],
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

	g_dbus_connection_register_object(
		this->connection,
		SERVICE_PATH,
		this->service_info->interfaces[0],
		&service_vtable,
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
		this->connection,
		CHARACTERISTIC_PATH,
		this->char_info->interfaces[0],
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

	return 0;
}

int64_t BLEServer::advertise_application() {
	GDBusProxy *adapter_proxy = this->create_adapter_proxy();
	if (adapter_proxy == nullptr) {
		printf("Error creating adapter proxy\n");
		return -1;
	}

	if (this->set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Powered",
																		g_variant_new_boolean(TRUE))) return -1;

	if (this->set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Discoverable",
																		g_variant_new_boolean(TRUE))) return -1;

	if (this->set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"DiscoverableTimeout",
																		g_variant_new_uint32(0))) return -1;

	if (this->set_proxy_property(adapter_proxy,
																		BLUEZ_ADAPTER_IFACE,
																		"Pairable",
																		g_variant_new_boolean(TRUE))) return -1;

	g_dbus_connection_call(
		this->connection,
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
		g_variant_new("(oa{sv})", ADVERTISING_PATH, nullptr),
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		nullptr,
		nullptr
	);

	return 0;
}

void BLEServer::start() {
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (this->init()) {
			printf("Failed to initialize BLEServer\n");
			this->cleanup();
			continue;
		}

		if (this->register_application() < 0) {
			printf("Failed to register application\n");
			this->cleanup();
			continue;
		}

		if (this->advertise_application() < 0) {
			printf("Failed to advertise application\n");
			this->cleanup();
			continue;
		}

		printf("BLE GATT server running...\n");
		if (this->main_loop) {
			g_main_loop_run(this->main_loop);
		}
	}
}

void BLEServer::cleanup() {
	if (this->app_info) g_dbus_node_info_unref(this->app_info);
  if (this->char_info) g_dbus_node_info_unref(this->char_info);
  if (this->adv_info) g_dbus_node_info_unref(this->adv_info);
  if (this->connection) g_object_unref(this->connection);

  this->app_info = nullptr;
  this->char_info = nullptr;
  this->adv_info = nullptr;
  this->connection = nullptr;

	if (this->main_loop) {
		g_main_loop_quit(this->main_loop);
		g_main_loop_unref(this->main_loop);
		this->main_loop = nullptr;
	}
}
