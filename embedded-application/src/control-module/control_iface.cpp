#include "control_module.hpp"
#include "control_iface.hpp"

Frame IControl::get_frame() {
  Frame frame =  ControlModule::get_instance().get_frame();
  return frame;
}

Obstacle IControl::get_obstacle() {
  Obstacle obstacle = ControlModule::get_instance().get_obstacle();
  return obstacle;
}

Audio IControl::get_audio_data() {
  Audio audio_data = ControlModule::get_instance().get_audio_data();
  return audio_data;
}

void IControl::set_frame(const Frame& frame) {
  ControlModule::get_instance().set_frame(frame);
}

void IControl::set_obstacle(const Obstacle& obstacle) {
  ControlModule::get_instance().set_obstacle(obstacle);
}

void IControl::set_audio_data(const Audio& data) {
  ControlModule::get_instance().set_audio_data(data);
}

void IControl::unlock_mutexes() {
  ControlModule::get_instance().unlock_mutexes();
}
