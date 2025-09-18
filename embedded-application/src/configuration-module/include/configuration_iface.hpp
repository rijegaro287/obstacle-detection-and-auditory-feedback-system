#pragma once

#include "configuration_module.hpp"

class IConfiguration {
private:
public:
  static void set_response_buffer(const string& buffer);
  static string get_response_buffer();
  static void process_command(const string& command);
  static void disconnect_audio_device();
};
