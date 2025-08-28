#ifdef __cplusplus
extern "C" {
#endif
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <gio/gio.h>

#define BLUEZ_SERVICE "org.bluez"
#define ADAPTER_PATH "/org/bluez/hci0"
#define ADAPTER_INTERFACE "org.bluez.Adapter1"
#define DEVICE_INTERFACE "org.bluez.Device1"

#define MAX_DEVICES 8
#define DEVICE_BUFFER_L 256
#define DEVICE_BUFFER_S 32

#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

typedef struct BlueZDevice_ {
	char name[DEVICE_BUFFER_L];
	char address[DEVICE_BUFFER_S];
} BlueZDevice;

void print_devices(BlueZDevice *devices, uint64_t device_count);

void clear_devices(BlueZDevice *devices);

void addr_to_path(char *addr, char *dest, uint64_t dest_size);

GDBusProxy* create_object_manager_proxy();

GDBusProxy* create_device_proxy(BlueZDevice device);

GVariant* get_managed_objects(GDBusProxy *proxy);

GVariant* get_device_property(GDBusProxy *proxy, const char *property);

bool is_paired(GDBusProxy *proxy);

bool is_connected(GDBusProxy *proxy);

int64_t start_discovery(GDBusProxy *proxy);

int64_t stop_discovery(GDBusProxy *proxy);

int64_t parse_devices(GVariant *devices_variant, BlueZDevice *dest, uint64_t max_devices);

int64_t scan_devices(BlueZDevice *dest, uint64_t max_devices);

int64_t pair_device(GDBusProxy *proxy);

int64_t connect_to_device(GDBusProxy *proxy);

int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid);

#ifdef __cplusplus
}
#endif
