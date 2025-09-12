#include "control_module.hpp"
#include "control_iface.hpp"
// #include "feedback_iface.hpp"
#include "image_capture_iface.hpp"
#include "obstacle_detection_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

ControlModule& ControlModule::get_instance() {
	static ControlModule instance;
	return instance;
}

ControlModule::ControlModule() {
  this->frame_mtx.lock();
  this->obstacle_mtx.lock();
  this->audio_mtx.lock();

  this->frame = Frame();
  this->obstacle = Obstacle();
  this->audio_data = Audio();
}

ControlModule::~ControlModule() {
  // delete this->obstacle_position;
  // delete this->audio_data;
}

Frame ControlModule::get_frame() {
  lock_guard<mutex> guard(this->frame_mtx);
  Frame frame = this->frame;
  this->frame = Frame();
  return frame;
}

Obstacle ControlModule::get_obstacle() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  Obstacle obstacle = this->obstacle;
  this->obstacle = Obstacle();
  return obstacle;
}

Audio ControlModule::get_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  Audio data = this->audio_data;
  this->audio_data = Audio();
  return data;
}

void ControlModule::set_frame(Frame frame) {
  lock_guard<mutex> guard(this->frame_mtx);
  this->frame = frame;
}

void ControlModule::set_obstacle(const Obstacle obstacle) {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle.azimuth = obstacle.azimuth;
  this->obstacle.elevation = obstacle.elevation;
  this->obstacle.meanDepth = obstacle.meanDepth;
}

void ControlModule::set_audio_data(const Audio& data) {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data->left_signal = data.left_signal;
  this->audio_data->right_signal = data.right_signal;
  this->audio_data->sample_rate = data.sample_rate;
}

void ControlModule::clear_obstacle_position() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle_position->azimuth = 0;
  this->obstacle_position->elevation = 0;
  this->obstacle_position->distance = 0;
}

void ControlModule::clear_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data->left_signal.clear();
  this->audio_data->right_signal.clear();
  this->audio_data->sample_rate = 0;
}

void ControlModule::start() {
  this->frame_mtx.unlock();
  this->obstacle_mtx.unlock();
  this->audio_mtx.unlock();
  /**this->obstacle_mtx.unlock();
  this->audio_mtx.unlock();

  // IFeedback::set_feedback_mode(VERBAL_MODE);
  IFeedback::set_feedback_mode(NON_VERBAL_MODE);

  vector<vector<float>> test_positions = {
    {  0.0f,   0.0f, 0.5f}, // FRONT
    {  0.0f,  10.0f, 0.5f}, // ABOVE
    {  0.0f, -10.0f, 0.5f}, // BELOW
    {340.0f, 	 0.0f, 0.5f}, // RIGHT
    { 25.0f,   0.0f, 0.5f}, // LEFT
    {340.0f,  10.0f, 0.5f}, // ABOVE RIGHT
    { 25.0f,  10.0f, 0.5f}, // ABOVE LEFT
    {340.0f, -10.0f, 0.5f}, // BELOW RIGHT
    { 25.0f, -10.0f, 0.5f}, // BELOW LEFT
  };

  bool mode = false;

  while (true) {
    if (mode) {
      IFeedback::set_feedback_mode(VERBAL_MODE);
    } 
    else {
      IFeedback::set_feedback_mode(NON_VERBAL_MODE);
    }

    for (const auto& position : test_positions) {
      IControl::set_obstacle({position[0], position[1], position[2]});
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    mode = !mode;
  }**/
}
