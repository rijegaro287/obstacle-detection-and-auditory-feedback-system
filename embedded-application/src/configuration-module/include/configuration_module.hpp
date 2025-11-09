#pragma once

/**
 * @file configuration_module.hpp
 * @brief Public API for the configuration module in charge of Bluetooth and
 * runtime control command processing.
 */

/**
 * @defgroup configuration_module Configuration Module
 * @brief Bluetooth configuration transport and runtime command processing.
 * @{
 */

#include "bt_controller.hpp"

#include <string>
#include <vector>

/**
 * @brief Canonical string commands exposed through the BLE configuration
 * channel.
 */
#define HEALTH_CHECK_COMMAND "health_check"
#define AUDIO_HEALTH_CHECK_COMMAND "audio_health_check"
#define START_DISCOVERY_COMMAND "start_discovery"
#define STOP_DISCOVERY_COMMAND "stop_discovery"
#define GET_DEVICES_COMMAND "get_devices"
#define PAIR_DEVICE_COMMAND "pair_device"
#define CONNECT_DEVICE_COMMAND "connect_device"
#define DISCONNECT_DEVICE_COMMAND "disconnect_device"
#define START_FEEDBACK_COMMAND "start_feedback"
#define STOP_FEEDBACK_COMMAND "stop_feedback"
#define SET_VOLUME_COMMAND "set_volume"
#define SET_FEEDBACK_MODE_COMMAND "set_feedback_mode"

/**
 * @enum COMMAND_CODE
 * @brief Numeric identifiers that help map incoming textual commands to their
 * corresponding handler implementation.
 */
enum COMMAND_CODE {
	HEALTH_CHECK_CODE,            /**< Validate configuration service liveness. */
  AUDIO_HEALTH_CHECK_CODE,      /**< Validate audio transport availability. */
  START_DISCOVERY_CODE,         /**< Begin Bluetooth device discovery. */
  STOP_DISCOVERY_CODE,          /**< Stop the Bluetooth discovery procedure. */
  GET_DEVICES_CODE,             /**< Retrieve the list of discovered devices. */
  PAIR_DEVICE_CODE,             /**< Pair with a discovered audio sink. */
	CONNECT_DEVICE_CODE,          /**< Connect to a paired audio sink. */
  DISCONNECT_DEVICE_CODE,       /**< Disconnect from the active audio sink. */
  START_FEEDBACK_CODE,          /**< Start the feedback processing chain. */
  STOP_FEEDBACK_CODE,           /**< Stop the feedback processing chain. */
  SET_VOLUME_CODE,              /**< Update feedback output volume. */
  SET_FEEDBACK_MODE_CODE        /**< Switch between verbal and non-verbal modes. */
};


using namespace std;

/**
 * @class ConfigModule
 * @ingroup configuration_module
 * @brief Singleton responsible for parsing BLE commands, orchestrating
 * Bluetooth audio connectivity, and delegating runtime requests to the control
 * subsystem.
 */
class ConfigModule {
public:
  ConfigModule(const ConfigModule&) = delete;
  ConfigModule& operator=(const ConfigModule&) = delete;
  ConfigModule(ConfigModule&&) = delete;
  ConfigModule& operator=(ConfigModule&&) = delete;

  /**
   * @brief Access the single configuration module instance.
   * @return Reference to the configuration module singleton.
   */
  static ConfigModule& get_instance();

  /**
   * @brief Persist the response that must be served to the BLE client.
   * @param buffer Plain-text payload to be returned on the next read request.
   */
  void set_response_buffer(const string& buffer);

  /**
   * @brief Consume and clear the pending BLE response buffer.
   * @return Most recent response payload awaiting transmission.
   */
  string get_response_buffer();

  /**
   * @brief Parse and dispatch an incoming command issued by the mobile
   * companion application.
   * @param command Command string following the expected tokenized format.
   */
  void process_command(const string& command);

  /**
   * @brief Disconnect from the currently connected Bluetooth audio device.
   * @return Status message describing the outcome of the operation.
   */
  string disconnect_device_command();

  /**
   * @brief Start the configuration module main loop, including BLE server
   * setup and command handling.
   */
  void start();
private:
  vector<BlueZDevice> found_devices; /**< Cache of devices found during discovery. */
  string response_buffer;            /**< Outgoing response buffer for BLE reads. */

  /**
   * @brief Split a tokenized command string using the requested delimiter.
   * @param s Input string to be broken apart.
   * @param delim Delimiter character.
   * @return Vector containing the resulting substrings.
   */
  vector<string> split(const string& s, char delim);

  /**
   * @brief Translate a textual command into the corresponding @ref COMMAND_CODE.
   * @param command Input command string.
   * @return Numeric code or -1 when the command is unknown.
   */
  int64_t map_command_to_code(const string& command);

  /**
   * @brief Handler for the health check command.
   * @return Static "OK" response when the module is operational.
   */
  string health_check_command();

  /**
   * @brief Handler that validates audio transport availability.
   * @return Status string describing audio health.
   */
  string audio_health_check_command();

  /**
   * @brief Handler that triggers Bluetooth device discovery.
   * @return Human-readable status message.
   */
  string start_discovery_command();

  /**
   * @brief Handler that halts Bluetooth discovery.
   * @return Human-readable status message.
   */
  string stop_discovery_command();

  /**
   * @brief Handler that exposes currently discovered devices.
   * @return Tokenized list of device identifiers to be consumed over BLE.
   */
  string get_devices_command();

  /**
   * @brief Handler for pairing requests submitted from the client.
   * @param args Command arguments (expects a single Bluetooth address).
   * @return Status string describing the result of the pairing attempt.
   */
  string pair_device_command(vector<string>& args);

  /**
   * @brief Handler for connection requests targeting a discovered device.
   * @param args Command arguments (expects a single Bluetooth address).
   * @return Status string describing the result of the connection attempt.
   */
  string connect_device_command(vector<string>& args);

  /**
   * @brief Handler that starts the multi-module feedback pipeline.
   * @return Status string describing the outcome.
   */
  string start_feedback_command();

  /**
   * @brief Handler that stops the multi-module feedback pipeline.
   * @return Status string describing the outcome.
   */
  string stop_feedback_command();

  /**
   * @brief Handler that updates the playback volume.
   * @param args Command arguments (expects a numeric volume percentage).
   * @return Status string describing the result of the change.
   */
  string set_volume_command(vector<string>& args);

  /**
   * @brief Handler that switches between verbal and non-verbal feedback modes.
   * @param args Command arguments (expects "verbal" or "non_verbal").
   * @return Status string describing the result of the change.
   */
  string set_feedback_mode_command(vector<string>& args);

  ConfigModule();
  ~ConfigModule() = default;
};

/// @}
