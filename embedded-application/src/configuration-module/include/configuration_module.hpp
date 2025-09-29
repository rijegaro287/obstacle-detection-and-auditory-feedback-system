#pragma once

#include "bt_controller.hpp"

#include <string>
#include <vector>

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

enum COMMAND_CODE {
	HEALTH_CHECK_CODE,
  AUDIO_HEALTH_CHECK_CODE,
  START_DISCOVERY_CODE,
  STOP_DISCOVERY_CODE,
  GET_DEVICES_CODE,
  PAIR_DEVICE_CODE,
	CONNECT_DEVICE_CODE,
  DISCONNECT_DEVICE_CODE,
  START_FEEDBACK_CODE,
  STOP_FEEDBACK_CODE,
  SET_VOLUME_CODE,
  SET_FEEDBACK_MODE_CODE
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
  string disconnect_device_command();

  void start();
private:
  vector<BlueZDevice> found_devices;
  string response_buffer;

  vector<string> split(const string& s, char delim);
  int64_t map_command_to_code(const string& command);

  string health_check_command();
  string audio_health_check_command();
  string start_discovery_command();
  string stop_discovery_command();
  string get_devices_command();
  string pair_device_command(vector<string>& args);
  string connect_device_command(vector<string>& args);
  string start_feedback_command();
  string stop_feedback_command();
  string set_volume_command(vector<string>& args);
  string set_feedback_mode_command(vector<string>& args);

  ConfigModule();
  ~ConfigModule() = default;
};
