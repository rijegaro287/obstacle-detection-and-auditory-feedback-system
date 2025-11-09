#pragma once

/**
 * @file configuration_iface.hpp
 * @brief Public façade that exposes the configuration module functionality to
 * other subsystems without leaking implementation details.
 */

#include "configuration_module.hpp"

/**
 * @class IConfiguration
 * @ingroup configuration_module
 * @brief Static bridge that forwards requests to the @ref ConfigModule
 * singleton while providing a narrow interface for the rest of the system.
 */
class IConfiguration {
private:
public:
  /**
   * @brief Store the response that should be delivered to the BLE client.
   * @param buffer Null-terminated string to expose in the configuration
   * characteristic.
   */
  static void set_response_buffer(const string& buffer);

  /**
   * @brief Retrieve and clear the pending BLE response payload.
   * @return Most recent configuration response string.
   */
  static string get_response_buffer();

  /**
   * @brief Process a command received by the BLE GATT characteristic.
   * @param command Tokenized command string.
   */
  static void process_command(const string& command);

  /**
   * @brief Disconnect from the active Bluetooth audio device, if any.
   */
  static void disconnect_audio_device();
};
