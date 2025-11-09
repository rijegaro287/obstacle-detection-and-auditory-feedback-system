/**
 * @file control_iface.cpp
 * @brief Implements the @ref IControl façade by forwarding to the
 * @ref ControlModule singleton.
 */

#include "control_module.hpp"
#include "control_iface.hpp"

/** See @ref IControl::get_frame. */
Frame IControl::get_frame() {
  Frame frame =  ControlModule::get_instance().get_frame();
  return frame;
}

/** See @ref IControl::set_frame. */
void IControl::set_frame(const Frame& frame) {
  ControlModule::get_instance().set_frame(frame);
}

/** See @ref IControl::get_obstacle. */
Obstacle IControl::get_obstacle() {
  Obstacle obstacle = ControlModule::get_instance().get_obstacle();
  return obstacle;
}

/** See @ref IControl::set_obstacle. */
void IControl::set_obstacle(const Obstacle& obstacle) {
  ControlModule::get_instance().set_obstacle(obstacle);
}

/** See @ref IControl::get_audio_data. */
Audio IControl::get_audio_data() {
  Audio audio_data = ControlModule::get_instance().get_audio_data();
  return audio_data;
}

/** See @ref IControl::set_audio_data. */
void IControl::set_audio_data(const Audio& data) {
  ControlModule::get_instance().set_audio_data(data);
}

/** See @ref IControl::start_feedback. */
void IControl::start_feedback() {
  ControlModule::get_instance().start_feedback();
}

/** See @ref IControl::stop_feedback. */
void IControl::stop_feedback() {
  ControlModule::get_instance().stop_feedback();
}

/** See @ref IControl::set_volume. */
void IControl::set_volume(uint64_t volume) {
  ControlModule::get_instance().set_volume(volume);
}

/** See @ref IControl::set_feedback_mode. */
void IControl::set_feedback_mode(FEEDBACK_MODES mode) {
  ControlModule::get_instance().set_feedback_mode(mode);
}

/** See @ref IControl::set_received_audio_commands. */
void IControl::set_received_audio_commands(bool status) {
  ControlModule::get_instance().set_received_audio_commands(status);
}

/** See @ref IControl::unlock_mutexes. */
void IControl::unlock_mutexes() {
  ControlModule::get_instance().unlock_mutexes();
}

/** See @ref IControl::add_capture_sample_start. */
void IControl::add_capture_sample_start() {
  PerformanceMonitor::get_instance().add_capture_sample_start();
}

/** See @ref IControl::add_capture_sample_end. */
void IControl::add_capture_sample_end() {
  PerformanceMonitor::get_instance().add_capture_sample_end();
}

/** See @ref IControl::add_detection_sample_start. */
void IControl::add_detection_sample_start() {
  PerformanceMonitor::get_instance().add_detection_sample_start();
}

/** See @ref IControl::add_detection_sample_end. */
void IControl::add_detection_sample_end() {
  PerformanceMonitor::get_instance().add_detection_sample_end();
}

/** See @ref IControl::add_feedback_sample_start. */
void IControl::add_feedback_sample_start() {
  PerformanceMonitor::get_instance().add_feedback_sample_start();
}

/** See @ref IControl::add_feedback_sample_end. */
void IControl::add_feedback_sample_end() {
  PerformanceMonitor::get_instance().add_feedback_sample_end();
}
