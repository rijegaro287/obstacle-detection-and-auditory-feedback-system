#pragma once

#include "bt_controller.hpp"

#include <string>
#include <vector>

#define HEALTH_CHECK_COMMAND "health_check"
#define SCAN_COMMAND "scan"
#define CONNECT_COMMAND "connect"

enum COMMAND_CODE {
	HEALTH_CHECK_CODE,
	SCAN_CODE,
	CONNECT_CODE,
};


using namespace std;

class ConfigModule {
public:
  ConfigModule(const ConfigModule&) = delete;
  ConfigModule& operator=(const ConfigModule&) = delete;
  ConfigModule(ConfigModule&&) = delete;
  ConfigModule& operator=(ConfigModule&&) = delete;

  static ConfigModule& get_instance();

  void set_response_buffer(const string& buffer);
  string get_response_buffer();
  void process_command(const string& command);
  void start();
private:
  vector<BlueZDevice> found_devices;
  string response_buffer;

  vector<string> split(const string& s, char delim);
  int64_t map_command_to_code(const string& command);

  string health_check_command();
  string scan_command();
  string connect_command(vector<string>& args);

  ConfigModule();
  ~ConfigModule() = default;
};
