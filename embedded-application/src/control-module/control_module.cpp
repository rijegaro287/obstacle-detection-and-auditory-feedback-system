#include "control_module.hpp"
#include "control_iface.hpp"
#include "feedback_iface.hpp"

#include <iostream>

ControlModule& ControlModule::get_instance() {
  test_t test_data;
  test_data.a = 5;
  test_data.b = 3.14;

  printf("Test Data - A: %d, B: %.2f\n", test_data.a, test_data.b);

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
  this->obstacle_mtx.unlock();

  this->set_obstacle_position({30.0, 10.0, 1.5});
  obstacle_position_t position = this->get_obstacle_position();

  printf("Obstacle Position - Azimuth: %.2f, Elevation: %.2f, Distance: %.2f\n",
         position.azimuth, position.elevation, position.distance);
}
