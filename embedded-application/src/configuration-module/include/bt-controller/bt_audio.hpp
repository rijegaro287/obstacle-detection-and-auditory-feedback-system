#pragma once

#include "bt_controller.hpp"


#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

class BTAudioController : public BTController {
public:
  BTAudioController(const BTAudioController&) = delete;
  BTAudioController& operator=(const BTAudioController&) = delete;
  BTAudioController(BTAudioController&&) = delete;
  BTAudioController& operator=(BTAudioController&&) = delete;

  static BTAudioController& get_instance();

	int64_t scan_devices(vector<BlueZDevice>& devices, uint64_t timeout_sec);
	BlueZDevice* find_device(vector<BlueZDevice>& devices, string address);
	int64_t pair_and_connect_device(BlueZDevice *device);


	void start();
	void cleanup(vector<BlueZDevice>& devices);
private:
	GMainLoop *main_loop;
	BlueZDevice *connected_device;
	
	int64_t start_discovery(GDBusProxy *proxy);
	int64_t stop_discovery(GDBusProxy *proxy);
	int64_t get_discovered_devices(vector<BlueZDevice>& devices);
	int64_t pair_device(GDBusProxy *proxy);
	int64_t connect_to_device(GDBusProxy *proxy);
	int64_t connect_to_device_profile(GDBusProxy *proxy, const char *uuid);
	
	bool is_paired(GDBusProxy *proxy);
	bool is_connected(GDBusProxy *proxy);
	bool get_boolean_value(GVariant *variant);

	BTAudioController();
	~BTAudioController() = default;
};
