#pragma once

#include <vector>
#include <mutex>

#include "control_iface.hpp"

using namespace std;

class control_module {
public:
  control_module(const control_module&) = delete;
  control_module& operator=(const control_module&) = delete;
  control_module(control_module&&) = delete;
  control_module& operator=(control_module&&) = delete;

  static control_module& get_instance();
  void set_obstacle_position(obstacle_position_t position);
  void set_audio_data(audio_data_t data);  

  obstacle_position_t get_obstacle_position();
  audio_data_t get_audio_data();

  void start();
private:
  obstacle_position_t *obstacle_position;
  audio_data_t *audio_data;

  mutex obstacle_mtx;
  mutex audio_mtx;

  control_module();
  ~control_module();
};
