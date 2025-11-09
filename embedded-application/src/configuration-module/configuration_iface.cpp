/**
 * @file configuration_iface.cpp
 * @brief Thin forwarding layer to expose @ref ConfigModule capabilities.
 */

#include "configuration_module.hpp"
#include "configuration_iface.hpp"

/** See @ref IConfiguration::set_response_buffer. */
void IConfiguration::set_response_buffer(const string& buffer) {
  ConfigModule::get_instance().set_response_buffer(buffer);
}

/** See @ref IConfiguration::get_response_buffer. */
string IConfiguration::get_response_buffer() {
  return ConfigModule::get_instance().get_response_buffer();
}

/** See @ref IConfiguration::process_command. */
void IConfiguration::process_command(const string& command) {
  ConfigModule::get_instance().process_command(command);
}

/** See @ref IConfiguration::disconnect_audio_device. */
void IConfiguration::disconnect_audio_device() {
  ConfigModule::get_instance().disconnect_device_command();
}
