#include "control_module.hpp"
#include "control_iface.hpp"

#include <iostream>

// #include "feedback_module.hpp"
// #include "transmission_module.hpp"

control_module& control_module::get_instance() {
	static control_module instance;
	return instance;
}

control_module::control_module() {
  this->obstacle_position = new obstacle_position_t;
  this->audio_data = new audio_data_t;

  this->obstacle_mtx.lock();
  this->audio_mtx.lock();
}

control_module::~control_module() {
  delete this->obstacle_position;
  delete this->audio_data;
}

void control_module::set_obstacle_position(obstacle_position_t position) {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle_position->azimuth = position.azimuth;
  this->obstacle_position->elevation = position.elevation;
  this->obstacle_position->distance = position.distance;
}

void control_module::set_audio_data(audio_data_t data) {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data->left_signal = data.left_signal;
  this->audio_data->right_signal = data.right_signal;
}

obstacle_position_t control_module::get_obstacle_position() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  return *this->obstacle_position;
}

audio_data_t control_module::get_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  return *this->audio_data;
}

void control_module::start() {
  this->obstacle_mtx.unlock();

  this->set_obstacle_position({30.0, 10.0, 1.5});
  obstacle_position_t position = this->get_obstacle_position();

  printf("Obstacle Position - Azimuth: %.2f, Elevation: %.2f, Distance: %.2f\n",
         position.azimuth, position.elevation, position.distance);
}
