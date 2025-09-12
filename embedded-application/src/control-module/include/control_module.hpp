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
  Audio get_audio_data();
  void set_frame(const Frame frame);
  void set_obstacle(const Obstacle obstacle);
  void set_audio_data(const Audio& data);

  void start();
private:
  Frame frame;
  Obstacle obstacle;
  Audio audio_data;

  mutex frame_mtx;
  mutex obstacle_mtx;
  mutex audio_mtx;

  ControlModule();
  ~ControlModule();
};
