#include "control_module.hpp"
#include "control_iface.hpp"

obstacle_position_t IControl::get_obstacle_position() {
  obstacle_position_t position = ControlModule::get_instance().get_obstacle_position();
  ControlModule::get_instance().clear_obstacle_position();
  return position;
}

audio_data_t IControl::get_audio_data() {
  audio_data_t audio_data = ControlModule::get_instance().get_audio_data();
  return audio_data;
}

void IControl::set_obstacle_position(const obstacle_position_t& position) {
  ControlModule::get_instance().set_obstacle_position(position);
}

void IControl::set_audio_data(const audio_data_t& data) {
  ControlModule::get_instance().set_audio_data(data);
}

void IControl::clear_obstacle_position() {
  ControlModule::get_instance().clear_obstacle_position();
}

void IControl::clear_audio_data() {
  ControlModule::get_instance().clear_audio_data();
}
