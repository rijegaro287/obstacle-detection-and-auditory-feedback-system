#include <iostream>
#include <thread>
#include <chrono>

#include "transmission_module.hpp"

#define BLUEZ_SERVICE "org.bluez"
#define ADAPTER_PATH "/org/bluez/hci0"
#define ADAPTER_INTERFACE "org.bluez.Adapter1"
#define DEVICE_INTERFACE "org.bluez.Device1"

#define CLIENT_SERVICE "com.odafs.transmission.client"

using namespace std;


void printDeviceInfo(const gchar* object_path, GVariant* ifaces_dict) {
	printf("Printing device info for: %s\n", object_path);

	// GVariantIter* objects_iter;
	// g_variant_get(result, "(a{oa{sa{sv}}})", &objects_iter);

	// const gchar* object_path;
	// GVariant* ifaces_dict;
	// while (g_variant_iter_next(objects_iter, "{oa{sa{sv}}}", &object_path, &ifaces_dict)) {
	// 	printDeviceInfo(object_path, ifaces_dict);
	// }

	const gchar* interface_name;
	GVariant* interface_dict;
	GVariantIter interface_iter;

	g_variant_iter_init(&interface_iter, ifaces_dict);
	while (g_variant_iter_next(&interface_iter, "{sa{sv}}", &interface_name, &interface_dict)) {
		printf("Interface: %s\n", interface_name);
	}
}

void listDiscoveredDevices() {
	std::cout << "\nDiscovered devices:" << std::endl;
	
	GError* error = nullptr;

	GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		"org.bluez",
		"/",
		"org.freedesktop.DBus.ObjectManager",
		NULL,
		&error
	);

	if (error) {
		printf("Error creating adapter proxy: %s\n", error->message);
		g_error_free(error);
		return ;
	}

	GVariant* result = g_dbus_proxy_call_sync(
		proxy,
		"GetManagedObjects",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error
	);

	if (error) {
		std::cerr << "Failed to get managed objects: " << error->message << std::endl;
		g_error_free(error);
		return;
	}

	
	GVariant *objects = g_variant_get_child_value(result, 0);
	if (strcmp(g_variant_get_type_string(objects), "a{oa{sa{sv}}}") != 0) {
		g_variant_unref(result);
		g_object_unref(proxy);
		return;
	}
	
	printf("---------------------------------------\n");
	GVariantIter objects_iter;
	const gchar *object_name;
	GVariant *ifaces_dict;

	g_variant_iter_init(&objects_iter, objects);
	while (g_variant_iter_next(&objects_iter, "{&o@a{sa{sv}}}", &object_name, &ifaces_dict)) {
		printf("* Device: %s\n", object_name);

		if (ifaces_dict == nullptr) return;

		if (strcmp(g_variant_get_type_string(ifaces_dict), "a{sa{sv}}") != 0) continue;

		GVariantIter iface_iter;
		const gchar *iface_name;
		GVariant *iface_dict;

		g_variant_iter_init(&iface_iter, ifaces_dict);
		while (g_variant_iter_next(&iface_iter, "{&s@a{sv}}", &iface_name, &iface_dict)) {
			if (strcmp(iface_name, DEVICE_INTERFACE) != 0) continue;
			printf("\t- Interface: %s\n", iface_name);
			
			GVariant *name_variant = g_variant_lookup_value(iface_dict, "Name", NULL);
			GVariant *addr_variant = g_variant_lookup_value(iface_dict, "Address", NULL);
			if (!name_variant || !addr_variant) continue;

			printf("\t\tAddress: %s\n", g_variant_get_string(addr_variant, NULL));
			printf("\t\tName: %s\n", g_variant_get_string(name_variant, NULL));
			
			if (addr_variant) g_variant_unref(addr_variant);
			if (name_variant) g_variant_unref(name_variant);
		}
		g_variant_unref(ifaces_dict);
	}
	g_variant_unref(result);
	g_object_unref(proxy);
	printf("---------------------------------------\n");
}

int main() {
	GError *error = NULL;

	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	GDBusProxy *adapter_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		BLUEZ_SERVICE,
		ADAPTER_PATH,
		ADAPTER_INTERFACE,
		NULL,
		&error
	);

	if (error) {
		printf("Error creating adapter proxy: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	while (true) {
		printf("=======================================\n");
		printf("Starting discovery...\n");
		
		GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (error) {
			printf("Error getting D-Bus connection: %s\n", error->message);
			g_error_free(error);
			break;
		}

		GVariant *result = g_dbus_proxy_call_sync(
			adapter_proxy,
			"StartDiscovery",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error
		);
	
		if (error) {
			printf("Error starting discovery: %s\n", error->message);
			g_object_unref(adapter_proxy);
			g_error_free(error);
			return -1;
		}
	
		if (result) g_variant_unref(result);
	
		this_thread::sleep_for(std::chrono::seconds(3));
	
		printf("Stopping discovery...\n");
		result = g_dbus_proxy_call_sync(
			adapter_proxy,
			"StopDiscovery",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error
		);
	
		if (error) {
			printf("Error stopping discovery: %s\n", error->message);
			g_object_unref(adapter_proxy);
			g_error_free(error);
		}
	
		if (result) g_variant_unref(result);

		listDiscoveredDevices();
		
		printf("=======================================\n");

		this_thread::sleep_for(std::chrono::seconds(1));
	}
	
	g_main_loop_run(loop);

	g_object_unref(adapter_proxy);
	g_main_loop_unref(loop);

	return 0;
}
