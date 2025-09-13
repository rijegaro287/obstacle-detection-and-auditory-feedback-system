#pragma once

#include <string>

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
  string response_buffer;

  ConfigModule();
  ~ConfigModule() = default;
};
