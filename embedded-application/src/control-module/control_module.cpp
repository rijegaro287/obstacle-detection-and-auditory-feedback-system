#include "control_module.hpp"
#include "control_iface.hpp"
#include "feedback_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

ControlModule& ControlModule::get_instance() {
	static ControlModule instance;
	return instance;
}

ControlModule::ControlModule() {
  this->obstacle_position = new obstacle_position_t;
  this->audio_data = new audio_data_t;

  this->obstacle_mtx.lock();
  this->audio_mtx.lock();
}

ControlModule::~ControlModule() {
  delete this->obstacle_position;
  delete this->audio_data;
}

void ControlModule::set_obstacle_position(obstacle_position_t position) {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle_position->azimuth = position.azimuth;
  this->obstacle_position->elevation = position.elevation;
  this->obstacle_position->distance = position.distance;
}

void ControlModule::set_audio_data(audio_data_t data) {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data->left_signal = data.left_signal;
  this->audio_data->right_signal = data.right_signal;
  this->audio_data->sample_rate = data.sample_rate;
}

obstacle_position_t ControlModule::get_obstacle_position() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  return *this->obstacle_position;
}

audio_data_t ControlModule::get_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  return *this->audio_data;
}

void ControlModule::start() {
  printf("Control Module started\n");
  this->obstacle_mtx.unlock();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    IControl::set_obstacle_position({30.0, 10.0, 1.5});
    printf("Obstacle changed!!!!!\n");
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    IControl::set_obstacle_position({0.0, 0.0, 0.0});
    printf("Obstacle changed!!!!!\n");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    IFeedback::set_feedback_mode(VERBAL_MODE);
    printf("Feedback mode changed!!!!!\n");

    std::this_thread::sleep_for(std::chrono::seconds(3));
    IFeedback::set_feedback_mode(NON_VERBAL_MODE);
    printf("Feedback mode changed!!!!!\n");

  }
  
}
