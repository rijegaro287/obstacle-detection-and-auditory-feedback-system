#include "control_module.hpp"
#include "control_iface.hpp"

Frame IControl::get_frame() {
  Frame frame =  ControlModule::get_instance().get_frame();
  return frame;
}

void IControl::set_frame(const Frame& frame) {
  ControlModule::get_instance().set_frame(frame);
}

Obstacle IControl::get_obstacle() {
  Obstacle obstacle = ControlModule::get_instance().get_obstacle();
  return obstacle;
}

void IControl::set_obstacle(const Obstacle& obstacle) {
  ControlModule::get_instance().set_obstacle(obstacle);
}

Audio IControl::get_audio_data() {
  Audio audio_data = ControlModule::get_instance().get_audio_data();
  return audio_data;
}

void IControl::set_audio_data(const Audio& data) {
  ControlModule::get_instance().set_audio_data(data);
}

void IControl::start_feedback() {
  ControlModule::get_instance().start_feedback();
}

void IControl::stop_feedback() {
  ControlModule::get_instance().stop_feedback();
}

void IControl::set_volume(uint64_t volume) {
  ControlModule::get_instance().set_volume(volume);
}

void IControl::set_feedback_mode(FEEDBACK_MODES mode) {
  ControlModule::get_instance().set_feedback_mode(mode);
}

void IControl::set_received_audio_commands(bool status) {
  ControlModule::get_instance().set_received_audio_commands(status);
}

void IControl::unlock_mutexes() {
  ControlModule::get_instance().unlock_mutexes();
}

void IControl::add_capture_sample_start() {
  ControlModule::get_instance().performance_monitor.add_capture_sample_start();
}

void IControl::add_capture_sample_end() {
  ControlModule::get_instance().performance_monitor.add_capture_sample_end();
}

void IControl::add_detection_sample_start() {
  ControlModule::get_instance().performance_monitor.add_detection_sample_start();
}

void IControl::add_detection_sample_end() {
  ControlModule::get_instance().performance_monitor.add_detection_sample_end();
}

void IControl::add_feedback_sample_start() {
  ControlModule::get_instance().performance_monitor.add_feedback_sample_start();
}

void IControl::add_feedback_sample_end() {
  ControlModule::get_instance().performance_monitor.add_feedback_sample_end();
}

void IControl::add_transmission_sample_start() {
  ControlModule::get_instance().performance_monitor.add_transmission_sample_start();
}

void IControl::add_transmission_sample_end(uint64_t signal_duration_ms) {
  ControlModule::get_instance().performance_monitor.add_transmission_sample_end(signal_duration_ms);
}
