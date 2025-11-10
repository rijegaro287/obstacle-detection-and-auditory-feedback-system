#pragma once

/**
 * @file bt_audio.hpp
 * @brief Bluetooth audio specific controller that extends the generic
 * @ref BTController with A2DP-focused helpers.
 */

#include "bt_controller.hpp"

#ifdef UNIT_TESTING
#include <functional>
#endif


/** @brief UUID representing the Advanced Audio Distribution Profile sink. */
#define A2DP_SINK_UUID "0000110b-0000-1000-8000-00805F9B34FB"

/**
 * @class BTAudioController
 * @brief Singleton that orchestrates discovery, pairing, and connection of
 * Bluetooth audio sinks using the BlueZ stack.
 */
class BTAudioController : public BTController {
public:
  BTAudioController(const BTAudioController&) = delete;
  BTAudioController& operator=(const BTAudioController&) = delete;
  BTAudioController(BTAudioController&&) = delete;
  BTAudioController& operator=(BTAudioController&&) = delete;

  /**
   * @brief Retrieve the singleton instance in charge of Bluetooth audio.
   * @return Reference to the controller.
   */
  static BTAudioController& get_instance();

	BlueZDevice *connected_device; /**< Pointer to the active audio device. */

	/**
	 * @brief Start Bluetooth device discovery on the default adapter.
	 * @return 0 on success, -1 on error.
	 */
	int64_t start_discovery();

	/**
	 * @brief Stop an ongoing discovery session.
	 * @return 0 on success, -1 on error.
	 */
	int64_t stop_discovery();

	/**
	 * @brief Populate a vector with the devices reported by BlueZ.
	 * @param devices Output container for the discovered devices.
	 * @return Number of devices found or -1 on error.
	 */
	int64_t get_discovered_devices(vector<BlueZDevice>& devices);

	/**
	 * @brief Find the index of a device by Bluetooth address.
	 * @param devices Collection to search in.
	 * @param address Bluetooth MAC address.
	 * @return Zero-based index or -1 when the device is absent.
	 */
	int64_t find_device_idx(vector<BlueZDevice>& devices, string address);

	/**
	 * @brief Attempt to pair with a given device.
	 * @param device Descriptor of the desired device.
	 * @return 0 on success, -1 on error.
	 */
	int64_t pair_device(BlueZDevice& device);

	/**
	 * @brief Attempt to connect to a previously paired device using A2DP.
	 * @param device Descriptor of the desired device.
	 * @return 0 on success, -1 on error.
	 */
	int64_t connect_device(BlueZDevice& device);

	/**
	 * @brief Disconnect from an active A2DP device.
	 * @param device Descriptor of the device to disconnect from.
	 * @return 0 on success, -1 on error.
	 */
	int64_t disconnect_device(BlueZDevice& device);

	/**
	 * @brief Inspect whether the provided device is paired with the adapter.
	 * @param device Descriptor of the device to query.
	 * @return true if paired, false otherwise.
	 */
	bool is_paired(BlueZDevice& device);

	/**
	 * @brief Inspect whether the provided device is connected.
	 * @param device Descriptor of the device to query.
	 * @return true if connected, false otherwise.
	 */
	bool is_connected(BlueZDevice& device);

	/**
	 * @brief Extract a boolean payload from a BlueZ variant response.
	 * @param variant Variant containing the boolean value.
	 * @return Extracted boolean (defaults to false on error).
	 */
	bool get_boolean_value(GVariant *variant);	

	/**
	 * @brief Run the Bluetooth audio controller loop (currently placeholder).
	 */
	void start();

	/**
	 * @brief Release resources after a discovery/connection run.
	 * @param devices Collection to clear.
	 */
	void cleanup(vector<BlueZDevice>& devices);
private:
	GMainLoop *main_loop; /**< GLib main loop used for async BlueZ interaction. */

	BTAudioController();
	~BTAudioController() = default;

#ifdef UNIT_TESTING
public:
	struct TestOverrides {
		std::function<int64_t()> start_discovery;
		std::function<int64_t()> stop_discovery;
		std::function<int64_t(vector<BlueZDevice>&)> get_discovered_devices;
		std::function<int64_t(BlueZDevice&)> pair_device;
		std::function<int64_t(BlueZDevice&)> connect_device;
		std::function<int64_t(BlueZDevice&)> disconnect_device;
		std::function<bool(BlueZDevice&)> is_paired;
		std::function<bool(BlueZDevice&)> is_connected;
		std::function<void(vector<BlueZDevice>&)> cleanup;
	};

	static TestOverrides test_overrides;
	static void reset_test_overrides();
#endif
};
