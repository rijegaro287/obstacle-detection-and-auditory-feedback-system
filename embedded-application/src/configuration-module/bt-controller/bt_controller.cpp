/**
 * @file bt_controller.cpp
 * @brief Implements generic GDBus helpers for interacting with BlueZ.
 */

#include "bt_controller.hpp"

#include <iostream>
#include <thread>
#include <chrono>

#ifdef UNIT_TESTING
GDBusConnection* BTController::test_system_connection = nullptr;

void BTController::set_test_system_connection(GDBusConnection* connection) {
	if (BTController::test_system_connection) {
		g_object_unref(BTController::test_system_connection);
	}
	BTController::test_system_connection = connection ? G_DBUS_CONNECTION(g_object_ref(connection)) : nullptr;
}

void BTController::clear_test_system_connection() {
	if (BTController::test_system_connection) {
		g_object_unref(BTController::test_system_connection);
		BTController::test_system_connection = nullptr;
	}
}
#endif

GDBusConnection* BTController::create_system_bus_connection() {
#ifdef UNIT_TESTING
	if (BTController::test_system_connection) {
		return G_DBUS_CONNECTION(g_object_ref(BTController::test_system_connection));
	}
#endif
	GError *error = nullptr;

	GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);

	if (!connection) {
		if (error) {
			printf("Error getting system bus connection: %s\n", error->message);
			g_error_free(error);
		} else {
			printf("Error getting system bus connection: unknown error\n");
		}
		return nullptr;
	}

	if (error) {
		printf("Error getting system bus connection: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return connection;
}

GDBusProxy* BTController::create_object_manager_proxy() {
	GError* error = nullptr;

#ifdef UNIT_TESTING
	if (BTController::test_system_connection) {
		GDBusProxy *object_manager_proxy = g_dbus_proxy_new_sync(
			BTController::test_system_connection,
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
#endif

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

GDBusProxy* BTController::create_properties_proxy() {
	GError* error = nullptr;

#ifdef UNIT_TESTING
	if (BTController::test_system_connection) {
		GDBusProxy *properties_proxy = g_dbus_proxy_new_sync(
			BTController::test_system_connection,
			G_DBUS_PROXY_FLAGS_NONE,
			nullptr,
			BLUEZ_SERVICE,
			"/",
			"org.freedesktop.DBus.Properties",
			nullptr,
			&error
		);

		if (error) {
			printf("Error creating properties proxy: %s\n", error->message);
			g_error_free(error);
			return nullptr;
		}

		return properties_proxy;
	}
#endif

	GDBusProxy *properties_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		nullptr,
		BLUEZ_SERVICE,
		"/",
		"org.freedesktop.DBus.Properties",
		nullptr,
		&error
	);

	if (error) {
		printf("Error creating properties proxy: %s\n", error->message);
		g_error_free(error);
		return nullptr;
	}

	return properties_proxy;
}

GDBusProxy* BTController::create_adapter_proxy() {
	GError* error = nullptr;

#ifdef UNIT_TESTING
	if (BTController::test_system_connection) {
		GDBusProxy *adapter_proxy = g_dbus_proxy_new_sync(
			BTController::test_system_connection,
			static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
				G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
				G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS),
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
#endif

GDBusProxy *adapter_proxy = g_dbus_proxy_new_for_bus_sync(
	G_BUS_TYPE_SYSTEM,
	static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_NONE),
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

GDBusProxy* BTController::create_device_proxy(BlueZDevice& device) {
	GError* error = nullptr;

	char device_path[BUFFER_SIZE_L] = {0};
	char device_addr[BUFFER_SIZE_S] = {0};

	this->addr_to_path(device.address, device_addr, BUFFER_SIZE_S);

	snprintf(device_path, BUFFER_SIZE_L, "%s/dev_%s", BLUEZ_ADAPTER_PATH, device_addr);
	device_path[BUFFER_SIZE_L - 1] = '\0';

#ifdef UNIT_TESTING
	if (BTController::test_system_connection) {
		GDBusProxy *device_proxy = g_dbus_proxy_new_sync(
			BTController::test_system_connection,
			static_cast<GDBusProxyFlags>(
				G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
				G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
				G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS),
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
#endif

	GDBusProxy *device_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		static_cast<GDBusProxyFlags>(
			G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
			G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
			G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS),
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

GVariant* BTController::get_managed_objects(GDBusProxy *proxy) {
	GError* error = nullptr;

	GVariant* result = g_dbus_proxy_call_sync(
		proxy,
		"GetManagedObjects",
		nullptr,
		G_DBUS_CALL_FLAGS_NONE,
		10000,
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

GVariant* BTController::get_proxy_property(GDBusProxy *proxy,
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
		10000,
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

int64_t BTController::set_proxy_property(GDBusProxy *proxy,
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
		10000,
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

int64_t BTController::parse_devices(GVariant *devices_variant, vector<BlueZDevice>& dest) {
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

			GVariant *name_variant = g_variant_lookup_value(iface_dict, "Name", nullptr);
			GVariant *addr_variant = g_variant_lookup_value(iface_dict, "Address", nullptr);
			if (!name_variant || !addr_variant) continue;

			BlueZDevice device;
			const char *name = g_variant_get_string(name_variant, nullptr);
			const char *address = g_variant_get_string(addr_variant, nullptr);
			
			strncpy(device.name, name, BUFFER_SIZE_L - 1);
			device.name[BUFFER_SIZE_L - 1] = '\0';

			strncpy(device.address, address, BUFFER_SIZE_S - 1);
			device.address[BUFFER_SIZE_S - 1] = '\0';

			dest.push_back(device);

			if (addr_variant) g_variant_unref(addr_variant);
			if (name_variant) g_variant_unref(name_variant);
		}
		g_variant_unref(ifaces_dict);
	}
	return device_count;
}

void BTController::addr_to_path(char *addr, char *dest, uint64_t dest_size) {
	for (uint64_t idx = 0; idx < dest_size - 1 && addr[idx] != '\0'; idx++) {
		char src_char = addr[idx];
		if (src_char == ':') src_char = '_';
		dest[idx] = src_char;
	}
	dest[dest_size - 1] = '\0';
}

void BTController::print_devices(vector<BlueZDevice>& devices) {
	for (size_t i = 0; i < devices.size(); i++) {
		BlueZDevice device = devices[i];
		printf("- Found device: %s (%s)\n", device.name, device.address);
	}
}

void BTController::clear_devices(vector<BlueZDevice>& devices) {
	for (size_t i = 0; i < devices.size(); i++) {
		memset(&devices[i], 0, sizeof(BlueZDevice));
	}
}
