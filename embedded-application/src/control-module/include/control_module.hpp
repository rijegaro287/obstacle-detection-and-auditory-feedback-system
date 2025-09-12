#pragma once

#include <vector>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "control_iface.hpp"

using namespace std;

class ControlModule {
public:
  ControlModule(const ControlModule&) = delete;
  ControlModule& operator=(const ControlModule&) = delete;
  ControlModule(ControlModule&&) = delete;
  ControlModule& operator=(ControlModule&&) = delete;

  static ControlModule& get_instance();
  Frame get_frame();
  Obstacle get_obstacle();
  audio_data_t get_audio_data();
  void set_frame(const Frame frame);
  void set_obstacle(const Obstacle obstacle);
  void set_audio_data(const audio_data_t& data);
  void clear_obstacle_position();
  void clear_audio_data();

  void start();
private:
  obstacle_position_t *obstacle_position;
  audio_data_t *audio_data;
  Frame frame;
  Obstacle obstacle;

  mutex obstacle_mtx;
  mutex audio_mtx;
  mutex frame_mtx;

  ControlModule();
  ~ControlModule();
};
