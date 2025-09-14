#include "configuration_module.hpp"
#include "configuration_iface.hpp"

void IConfiguration::set_response_buffer(const string& buffer) {
  ConfigModule::get_instance().set_response_buffer(buffer);
}

string IConfiguration::get_response_buffer() {
  return ConfigModule::get_instance().get_response_buffer();
}

void IConfiguration::process_command(const string& command) {
  ConfigModule::get_instance().process_command(command);
}