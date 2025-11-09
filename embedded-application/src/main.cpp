/**
 * @file main.cpp
 * @brief Entry point that wires together all application modules and drives
 * the multi-threaded execution of the embedded obstacle detection pipeline.
 */

#include "control_module.hpp"
#include "configuration_module.hpp"
#include "image_capture_module.hpp"
#include "obstacle_detection_module.hpp"
#include "feedback_module.hpp"
#include "transmission_module.hpp"

#include <thread>
#include <pthread.h>

/**
 * @brief Attempt to set the scheduling policy and priority of a C++ thread.
 *
 * @param thr Reference to the thread whose native handle will be adjusted.
 * @param policy POSIX scheduling policy (for example `SCHED_FIFO`).
 * @param priority Priority value compatible with the selected policy.
 * @return true when the priority is successfully applied, false otherwise.
 */
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

/**
 * @brief Program entry point that creates one thread per functional module
 * and blocks until each worker thread completes.
 */
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

  config_thread.join();
  control_thread.join();
  capture_thread.join();
  detection_thread.join();
  feedback_thread.join();
  transmission_thread.join();

  return 0;
}
