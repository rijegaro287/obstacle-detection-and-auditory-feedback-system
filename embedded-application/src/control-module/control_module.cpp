#include "control_module.hpp"
#include "control_iface.hpp"
#include "image_capture_iface.hpp"
#include "obstacle_detection_iface.hpp"
#include "transmission_iface.hpp"
#include "feedback_iface.hpp"

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

}

Frame ControlModule::get_frame() {
  lock_guard<mutex> guard(this->frame_mtx);
  Frame frame = this->frame;
  this->frame = Frame();
  return frame;
}

void ControlModule::set_frame(Frame frame) {
  lock_guard<mutex> guard(this->frame_mtx);
  this->frame = frame;
}

Obstacle ControlModule::get_obstacle() {
  lock_guard<mutex> guard(this->obstacle_mtx);
  Obstacle obstacle = this->obstacle;
  this->obstacle = Obstacle();
  return obstacle;
}

void ControlModule::set_obstacle(const Obstacle obstacle) {
  lock_guard<mutex> guard(this->obstacle_mtx);
  this->obstacle.azimuth = obstacle.azimuth;
  this->obstacle.elevation = obstacle.elevation;
  this->obstacle.meanDepth = obstacle.meanDepth;
}

Audio ControlModule::get_audio_data() {
  lock_guard<mutex> guard(this->audio_mtx);
  Audio data = this->audio_data;
  this->audio_data = Audio();
  return data;
}

void ControlModule::set_audio_data(const Audio& data) {
  lock_guard<mutex> guard(this->audio_mtx);
  this->audio_data.left_signal = data.left_signal;
  this->audio_data.right_signal = data.right_signal;
  this->audio_data.sample_rate = data.sample_rate;
}

void ControlModule::start_feedback() {
  IImageCapture::start_capture();
  IObstacleDetection::start_detection();
  IFeedback::start_feedback();
  ITransmission::start_transmission();
}

void ControlModule::stop_feedback() {
  IImageCapture::stop_capture();
  IObstacleDetection::stop_detection();
  IFeedback::stop_feedback();
  ITransmission::stop_transmission();
}

void ControlModule::set_volume(uint64_t volume) {
  IFeedback::set_volume(volume);
}

void ControlModule::set_feedback_mode() {
  IFeedback::set_feedback_mode(NON_VERBAL_MODE);
}

void ControlModule::unlock_mutexes() {
  this->frame_mtx.unlock();
  this->obstacle_mtx.unlock();
  this->audio_mtx.unlock();
}

void ControlModule::start() {
  this->unlock_mutexes();
}
