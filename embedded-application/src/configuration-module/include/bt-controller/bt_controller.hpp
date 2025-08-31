#pragma once

#include <cstdint>
#include <vector>

#include <gio/gio.h>

#define BLUEZ_SERVICE "org.bluez"
#define BLUEZ_ADAPTER_IFACE "org.bluez.Adapter1"
#define BLUEZ_DEVICE_IFACE "org.bluez.Device1"
#define BLUEZ_ADAPTER_PATH "/org/bluez/hci0"

#define BUFFER_SIZE_L 256
#define BUFFER_SIZE_S 32

using namespace std;

typedef struct BlueZDevice_ {
	char name[BUFFER_SIZE_L];
	char address[BUFFER_SIZE_S];
} BlueZDevice;

class BTController {
public:
  static GDBusConnection* create_system_bus_connection();
  static GDBusProxy* create_object_manager_proxy();
  static GDBusProxy* create_properties_proxy();
  static GDBusProxy* create_adapter_proxy();
  static GDBusProxy* create_device_proxy(BlueZDevice device);
  static GVariant* get_managed_objects(GDBusProxy *proxy);
  static GVariant* get_proxy_property(GDBusProxy *proxy, const char *interface, const char *property);
  static int64_t set_proxy_property(GDBusProxy *proxy, const char *interface, const char *property, GVariant *value);
  static int64_t parse_devices(GVariant *devices_variant, vector<BlueZDevice>& dest);
  
  static void addr_to_path(char *addr, char *dest, uint64_t dest_size);
  static void print_devices(vector<BlueZDevice>& devices);
  static void clear_devices(vector<BlueZDevice>& devices);
};
