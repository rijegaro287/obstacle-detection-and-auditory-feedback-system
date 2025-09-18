#include "control_module.hpp"
#include "configuration_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_module.hpp"
#include "feedback_module.hpp"
#include "transmission_module.hpp"

#include <thread>
#include <pthread.h>

bool set_thread_priority(std::thread &thr, int policy, int priority) {
  pthread_t handle = thr.native_handle();
  struct sched_param sch;
  sch.sched_priority = priority;
  int result = pthread_setschedparam(handle, policy, &sch);
  if (result != 0) {
    std::cerr << "Failed to set thread priority: " << strerror(result) << "\n";
    return false;
  }
  return true;
}

int main() {
  ConfigModule& config_module = ConfigModule::get_instance();
  ControlModule& control_module = ControlModule::get_instance();
  ImageCaptureModule& capture_module = ImageCaptureModule::get_instance();
  ObstacleDetectionModule& detection_module = ObstacleDetectionModule::get_instance();
  FeedbackModule& feedback_module = FeedbackModule::get_instance();
  TransmissionModule& transmission_module = TransmissionModule::get_instance();

  thread config_thread(&ConfigModule::start, &config_module);
  thread control_thread(&ControlModule::start, &control_module);
  thread capture_thread(&ImageCaptureModule::start, &capture_module);
  thread detection_thread(&ObstacleDetectionModule::start, &detection_module);
  thread feedback_thread(&FeedbackModule::start, &feedback_module);
  thread transmission_thread(&TransmissionModule::start, &transmission_module);

  set_thread_priority(config_thread, SCHED_RR, 80);
  set_thread_priority(control_thread, SCHED_RR, 70);
  set_thread_priority(capture_thread, SCHED_RR, 60);
  set_thread_priority(detection_thread, SCHED_RR, 60);
  set_thread_priority(feedback_thread, SCHED_RR, 60);
  set_thread_priority(transmission_thread, SCHED_RR, 80);

  config_thread.join();
  control_thread.join();
  capture_thread.join();
  detection_thread.join();
  feedback_thread.join();
  transmission_thread.join();

  return 0;
}
