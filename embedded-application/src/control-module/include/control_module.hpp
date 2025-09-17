#pragma once

#include "control_iface.hpp"

#include <vector>
#include <mutex>
#include <opencv2/opencv.hpp>

using namespace std;

class ControlModule {
public:
  ControlModule(const ControlModule&) = delete;
  ControlModule& operator=(const ControlModule&) = delete;
  ControlModule(ControlModule&&) = delete;
  ControlModule& operator=(ControlModule&&) = delete;

  static ControlModule& get_instance();
  Frame get_frame();
  void set_frame(const Frame frame);
  
  Obstacle get_obstacle();
  void set_obstacle(const Obstacle obstacle);
  
  Audio get_audio_data();
  void set_audio_data(const Audio& data);
  
  void start_feedback();
  void stop_feedback();
  void set_volume(uint64_t volume);
  void set_feedback_mode();

  void unlock_mutexes();

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
