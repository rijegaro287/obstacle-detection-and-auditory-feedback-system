#pragma once

#include "ble_server.hpp"
#include "bt_audio.hpp"

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

// class Characteristic {
// public:
//   Characteristic(GDBusConnection* conn, const string& uuid, const string& path);
//   ~Characteristic() = default;

//   vector<uint8_t> get_value();
//   void set_value(const vector<uint8_t>& value);
//   void notify_value_changed();
// private:
//   string char_uuid;
//   string object_path;
//   GDBusConnection* connection;
//   vector<uint8_t> value;
// };

// class Service {
// public:
//   Service(GDBusConnection* conn, const string& uuid, const string& path);
//   ~Service() = default;

//   void add_characteristic(Characteristic* characteristic);
// private:
//   string service_uuid;
//   string object_path;
//   GDBusConnection* connection;
//   vector<Characteristic*> characteristics;
// };
