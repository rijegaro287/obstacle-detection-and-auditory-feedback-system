#pragma once

class ConfigModule {
public:
  ConfigModule(const ConfigModule&) = delete;
  ConfigModule& operator=(const ConfigModule&) = delete;
  ConfigModule(ConfigModule&&) = delete;
  ConfigModule& operator=(ConfigModule&&) = delete;

  static ConfigModule& get_instance();

  void start();
private:
  ConfigModule();
  ~ConfigModule() = default;
};
