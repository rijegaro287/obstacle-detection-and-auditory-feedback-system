#pragma once

#include <vector>
#include <mutex>

#include "control_iface.hpp"

using namespace std;

class ControlModule {
public:
  ControlModule(const ControlModule&) = delete;
  ControlModule& operator=(const ControlModule&) = delete;
  ControlModule(ControlModule&&) = delete;
  ControlModule& operator=(ControlModule&&) = delete;

  static ControlModule& get_instance();
  obstacle_position_t get_obstacle_position();
  audio_data_t get_audio_data();
  void set_obstacle_position(const obstacle_position_t& position);
  void set_audio_data(const audio_data_t& data);
  void clear_obstacle_position();
  void clear_audio_data();

  void start();
private:
  obstacle_position_t *obstacle_position;
  audio_data_t *audio_data;

  mutex obstacle_mtx;
  mutex audio_mtx;

  ControlModule();
  ~ControlModule();
};
