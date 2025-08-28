#include <iostream>
#include <thread>
#include <chrono>

#include "transmission_module.hpp"

#define BLUEZ_SERVICE "org.bluez"
#define ADAPTER_PATH "/org/bluez/hci0"
#define ADAPTER_INTERFACE "org.bluez.Adapter1"
#define DEVICE_INTERFACE "org.bluez.Device1"

#define MAX_DEVICES 8
#define DEVICE_BUFFER_L 256
#define DEVICE_BUFFER_S 24

using namespace std;

typedef struct BlueZDevice_ {
	char name[DEVICE_BUFFER_L];
	char address[DEVICE_BUFFER_S];
} BlueZDevice;

void print_devices(BlueZDevice *devices, uint64_t device_count) {
	for (size_t i = 0; i < device_count; i++) {
		BlueZDevice device = devices[i];
		printf("- Found device: %s (%s)\n", device.name, device.address);
	}
}

void clear_devices(BlueZDevice *devices) {
	if (devices == NULL) return;
	for (int i = 0; i < MAX_DEVICES; i++) {
		memset(&devices[i], 0, sizeof(BlueZDevice));
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

GDBusProxy* create_object_manager_proxy() {
	GError* error = NULL;

	GDBusProxy *object_manager_proxy = g_dbus_proxy_new_for_bus_sync(
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
		printf("Error creating object_manager_proxy: %s\n", error->message);
		g_error_free(error);
		return NULL;
	}

	return object_manager_proxy;
}

GDBusProxy* create_device_proxy(BlueZDevice device) {
	GError* error = NULL;

	char device_path[DEVICE_BUFFER_L] = {0};
	char device_addr[DEVICE_BUFFER_S] = {0};

	addr_to_path(device.address, device_addr, DEVICE_BUFFER_S);

	snprintf(device_path, DEVICE_BUFFER_L, "%s/dev_%s", ADAPTER_PATH, device_addr);
	device_path[DEVICE_BUFFER_L - 1] = '\0';

	printf("Creating device proxy for %s\n", device_path);

	GDBusProxy *device_proxy = g_dbus_proxy_new_for_bus_sync(
		G_BUS_TYPE_SYSTEM,
		G_DBUS_PROXY_FLAGS_NONE,
		NULL,
		BLUEZ_SERVICE,
		device_path,
		DEVICE_INTERFACE,
		NULL,
		&error
	);

	if (error) {
		printf("Error creating device proxy: %s\n", error->message);
		g_error_free(error);
		return NULL;
	}

	return device_proxy;
}

GVariant* get_managed_objects(GDBusProxy *proxy) {
	GError* error = NULL;

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
		printf("Error getting managed objects: %s\n", error->message);
		g_error_free(error);
		return NULL;
	}

	return result;
}

GVariant* get_device_property(GDBusProxy *proxy, const char *property) {
	if (proxy == NULL || property == NULL) return NULL;

	GError* error = NULL;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"org.freedesktop.DBus.Properties.Get",
		g_variant_new("(ss)", DEVICE_INTERFACE, property),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
		&error
	);

	if (error) {
		printf("Error getting device property %s: %s\n", property, error->message);
		g_error_free(error);
		return NULL;
	}

	return result;
}

bool is_paired(GDBusProxy *proxy) {
	if (proxy == NULL) return false;

	GVariant *result = get_device_property(proxy, "Paired");
	if (result == NULL) return false;

	GVariant *value = NULL;
	g_variant_get(result, "(v)", &value);
 	gboolean paired = g_variant_get_boolean(value);

	g_variant_unref(value);
	g_variant_unref(result);

	return paired;
}

bool is_connected(GDBusProxy *proxy) { 
	if (proxy == NULL) return false;

	GVariant *result = get_device_property(proxy, "Connected");
	if (result == NULL) return false;

	GVariant *value = NULL;
	g_variant_get(result, "(v)", &value);
 	gboolean connected = g_variant_get_boolean(value);

	g_variant_unref(value);
	g_variant_unref(result);

	return connected;
}

int64_t start_discovery(GDBusProxy *proxy) {
	GError *error = NULL;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"StartDiscovery",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
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
	GError *error = NULL;

	GVariant *result = g_dbus_proxy_call_sync(
			proxy,
			"StopDiscovery",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
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

int64_t parse_devices(GVariant *devices_variant, BlueZDevice *dest, uint64_t max_devices) {
	int64_t device_count = 0;
	if (strcmp(g_variant_get_type_string(devices_variant), "a{oa{sa{sv}}}") != 0) return -1;

	GVariantIter devices_iter;
	const gchar *device_name;
	GVariant *ifaces_dict;

	g_variant_iter_init(&devices_iter, devices_variant);
	while (g_variant_iter_next(&devices_iter, "{&o@a{sa{sv}}}", &device_name, &ifaces_dict)) {
		if (ifaces_dict == NULL) return -1;
		if (strcmp(g_variant_get_type_string(ifaces_dict), "a{sa{sv}}") != 0) continue;

		GVariantIter iface_iter;
		const gchar *iface_name;
		GVariant *iface_dict;

		g_variant_iter_init(&iface_iter, ifaces_dict);
		uint64_t device_idx = 0;
		while (g_variant_iter_next(&iface_iter, "{&s@a{sv}}", &iface_name, &iface_dict)) {
			if (strcmp(iface_name, DEVICE_INTERFACE) != 0) continue;
			if (device_count + device_idx >= max_devices) break;

			GVariant *name_variant = g_variant_lookup_value(iface_dict, "Name", NULL);
			GVariant *addr_variant = g_variant_lookup_value(iface_dict, "Address", NULL);
			if (!name_variant || !addr_variant) continue;

			const char *name = g_variant_get_string(name_variant, NULL);
			const char *address = g_variant_get_string(addr_variant, NULL);

			strncpy(dest[device_count + device_idx].name, name, DEVICE_BUFFER_L - 1);
			dest[device_count + device_idx].name[DEVICE_BUFFER_L - 1] = '\0';

			strncpy(dest[device_count + device_idx].address, address, DEVICE_BUFFER_S - 1);
			dest[device_count + device_idx].address[DEVICE_BUFFER_S - 1] = '\0';

			device_idx++;

			if (addr_variant) g_variant_unref(addr_variant);
			if (name_variant) g_variant_unref(name_variant);
		}
		device_count += device_idx;
		g_variant_unref(ifaces_dict);
	}
	return device_count;
}

int64_t scan_devices(BlueZDevice *dest, uint64_t max_devices) {
	GError* error = NULL;

	GDBusProxy *object_manager_proxy = create_object_manager_proxy();
	if (object_manager_proxy == NULL) return -1;

	GVariant *managed_objects = get_managed_objects(object_manager_proxy);
	if (managed_objects == NULL) {
		g_object_unref(object_manager_proxy);
		return -1;
	};

	GVariant *managed_devices = g_variant_get_child_value(managed_objects, 0);
	if (managed_devices == NULL) {
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
	if (proxy == NULL) return -1;
	GError* error = NULL;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"Pair",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
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
	if (proxy == NULL) return -1;
	GError* error = NULL;

	GVariant *result = g_dbus_proxy_call_sync(
		proxy,
		"Connect",
		NULL,
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		NULL,
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
		printf("Error creating adapter object_manager_proxy: %s\n", error->message);
		g_error_free(error);
		return -1;
	}

	BlueZDevice devices[MAX_DEVICES] = {0};
	int64_t status = 0;
	while (true) {
		printf("=======================================\n");
		printf("Starting discovery...\n");
		if (start_discovery(adapter_proxy) < 0) {
			printf("Failed to start discovery\n");
			status = -1;
			break;
		}

		this_thread::sleep_for(std::chrono::seconds(3));

		printf("Stopping discovery...\n");
		if (stop_discovery(adapter_proxy) < 0) {
			printf("Failed to stop discovery\n");
			status = -1;
			break;
		}

		int64_t device_count = scan_devices(devices, MAX_DEVICES);
		if (device_count < 0) {
			printf("Failed to scan devices\n");
			status = -1;
			break;
		}

		print_devices(devices, device_count);

		GDBusProxy *device_proxy = NULL;
		for (uint64_t idx = 0; idx < device_count; idx++) {
			BlueZDevice device = devices[idx];
			if (strcmp(device.name, "QCY H3") == 0) {
				printf("Connecting to device: %s (%s)\n", device.name, device.address);

				device_proxy = create_device_proxy(device);
				if (device_proxy == NULL) {
					printf("Failed to create device proxy\n");
					status = -1;
					break;
				}

				if (!is_paired(device_proxy)) {
					printf("Device not paired, pairing...\n");
					if (pair_device(device_proxy) < 0) {
						printf("Failed to pair to device\n");
						status = -1;
						break;
					}
				}
				printf("Device paired, connecting...\n");
				
				if (!is_connected(device_proxy)) {
					printf("Device not connected, connecting...\n");
					if (connect_to_device(device_proxy) < 0) {
						printf("Failed to connect to device\n");
						status = -1;
						break;
					}
				}
				printf("Device connected successfully!\n");
			}
		}

		if (device_proxy) g_object_unref(device_proxy);
		clear_devices(devices);
		printf("=======================================\n");
		this_thread::sleep_for(std::chrono::seconds(1));
	}

	g_main_loop_unref(loop);
	g_object_unref(adapter_proxy);

	return status;
}


