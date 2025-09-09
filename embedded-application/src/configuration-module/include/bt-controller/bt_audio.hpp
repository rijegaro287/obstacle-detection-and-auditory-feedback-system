#pragma once

#include "bt_controller.hpp"

#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

class BTAudioController : public BTController {
public:
	static void start();
	static void cleanup(vector<BlueZDevice>& devices);
private:
	static GMainLoop *main_loop;
	static BlueZDevice *connected_device;
	
	static int64_t init();
	static int64_t start_discovery(GDBusProxy *proxy);
	static int64_t stop_discovery(GDBusProxy *proxy);
	static int64_t get_discovered_devices(vector<BlueZDevice>& devices);
	static int64_t scan_devices(vector<BlueZDevice>& devices, uint64_t timeout_sec);
	static int64_t pair_device(GDBusProxy *proxy);
	static int64_t connect_to_device(GDBusProxy *proxy);
	static int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid);
	static int64_t connect_and_pair_device(BlueZDevice *device);
	
	static BlueZDevice* find_device(vector<BlueZDevice>& devices, const char *name);
	static bool is_paired(GDBusProxy *proxy);
	static bool is_connected(GDBusProxy *proxy);
	static bool get_boolean_value(GVariant *variant);
};
