#pragma once

#include "bt_controller.hpp"

#define MAX_DEVICES 8
#define BUFFER_SIZE_L 256
#define BUFFER_SIZE_S 32

#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

typedef struct bluez_device_t_ {
	char name[BUFFER_SIZE_L];
	char address[BUFFER_SIZE_S];
} bluez_device_t;

void print_devices(bluez_device_t *devices, uint64_t device_count);

void clear_devices(bluez_device_t *devices);

void addr_to_path(char *addr, char *dest, uint64_t dest_size);

GDBusConnection* create_system_bus_connection();

GDBusProxy* create_object_manager_proxy();

GDBusProxy* create_properties_proxy();

GDBusProxy* create_adapter_proxy();

GDBusProxy* create_device_proxy(bluez_device_t device);

GVariant* get_managed_objects(GDBusProxy *proxy);

GVariant* get_proxy_property(GDBusProxy *proxy, const char *interface, const char *property);

int64_t set_proxy_property(GDBusProxy *proxy, const char *interface, const char *property, GVariant *value);

int64_t start_discovery(GDBusProxy *proxy);

int64_t stop_discovery(GDBusProxy *proxy);

int64_t parse_devices(GVariant *devices_variant, bluez_device_t *dest, uint64_t max_devices);

int64_t scan_devices(bluez_device_t *dest, uint64_t max_devices);

int64_t pair_device(GDBusProxy *proxy);

int64_t connect_to_device(GDBusProxy *proxy);

int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid);
