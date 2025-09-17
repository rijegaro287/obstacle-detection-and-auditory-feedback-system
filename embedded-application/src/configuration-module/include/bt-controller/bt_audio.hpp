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

	BlueZDevice *connected_device;

	int64_t start_discovery();
	int64_t stop_discovery();
	int64_t get_discovered_devices(vector<BlueZDevice>& devices);
	int64_t find_device_idx(vector<BlueZDevice>& devices, string address);
	int64_t pair_device(BlueZDevice& device);
	int64_t connect_device(BlueZDevice& device);
	int64_t disconnect_device(BlueZDevice& device);

	bool is_paired(BlueZDevice& device);
	bool is_connected(BlueZDevice& device);
	bool get_boolean_value(GVariant *variant);	

	void start();
	void cleanup(vector<BlueZDevice>& devices);
private:
	GMainLoop *main_loop;
	
	BTAudioController();
	~BTAudioController() = default;
};
