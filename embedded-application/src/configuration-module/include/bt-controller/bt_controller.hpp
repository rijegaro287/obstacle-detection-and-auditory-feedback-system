#pragma once

#include <cstdint>
#include <gio/gio.h>

#define BLUEZ_SERVICE "org.bluez"
#define BLUEZ_ADAPTER_IFACE "org.bluez.Adapter1"
#define BLUEZ_DEVICE_IFACE "org.bluez.Device1"
#define BLUEZ_ADAPTER_PATH "/org/bluez/hci0"

#define GATT_MANAGER_IFACE "org.bluez.GattManager1"
#define GATT_APPLICATION_IFACE "org.bluez.GattApplication1"
#define GATT_SERVICE_IFACE "org.bluez.GattService1"
#define GATT_CHARACTERISTIC_IFACE "org.bluez.GattCharacteristic1"
#define LE_ADVERTISING_MANAGER_IFACE "org.bluez.LEAdvertisingManager1"

#define DEVICE_NAME "MyBluetoothDevice"
#define APP_SERVICE "com.example.app"
#define APP_PATH "/com/example/app"
#define SERVICE_PATH "/com/example/app/service0"
#define CHARACTERISTIC_PATH "/com/example/app/service0/char0"
#define ADVERTISING_PATH "/com/example/app/advertising0"

#define SERVICE_NAME "com.example.service"
#define SERVICE_UUID "12301101-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_UUID "12301102-0000-1000-8000-00805F9B34FB"

#define MAX_DEVICES 8
#define BLUEZ_DEVICE_BUFFER_L 256
#define BLUEZ_DEVICE_BUFFER_S 32

#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

typedef struct bluez_device_t_ {
	char name[BLUEZ_DEVICE_BUFFER_L];
	char address[BLUEZ_DEVICE_BUFFER_S];
} bluez_device_t;

void print_devices(bluez_device_t *devices, uint64_t device_count);

void clear_devices(bluez_device_t *devices);

void addr_to_path(char *addr, char *dest, uint64_t dest_size);

GDBusConnection* create_system_bus_connection();

GDBusProxy* create_object_manager_proxy();

GDBusProxy* create_gatt_manager_proxy();

GDBusProxy* create_properties_proxy();

GDBusProxy* create_adapter_proxy();

GDBusProxy* create_device_proxy(bluez_device_t device);

GVariant* get_managed_objects(GDBusProxy *proxy);

GVariant* get_proxy_property(GDBusProxy *proxy, const char *interface, const char *property);

int64_t set_proxy_property(GDBusProxy *proxy, const char *interface, const char *property, GVariant *value);

// bool is_paired(GDBusProxy *proxy);

// bool is_connected(GDBusProxy *proxy);

int64_t start_discovery(GDBusProxy *proxy);

int64_t stop_discovery(GDBusProxy *proxy);

int64_t parse_devices(GVariant *devices_variant, bluez_device_t *dest, uint64_t max_devices);

int64_t scan_devices(bluez_device_t *dest, uint64_t max_devices);

int64_t pair_device(GDBusProxy *proxy);

int64_t connect_to_device(GDBusProxy *proxy);

int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid);
