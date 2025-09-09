#include "control_module.hpp"
#include "feedback_module.hpp"
#include "transmission_module.hpp"
#include "configuration_module.hpp"

#include <thread>
#include <pthread.h>

int main() {
  // ConfigModule& config_module = ConfigModule::get_instance();
  // config_module.start();

  ControlModule& control_module = ControlModule::get_instance();
  FeedbackModule& feedback_module = FeedbackModule::get_instance();
  TransmissionModule& transmission_module = TransmissionModule::get_instance();

  printf("Starting Control, Feedback, and Transmission Modules...\n");
  thread control_thread(&ControlModule::start, &control_module);
  thread feedback_thread(&FeedbackModule::start, &feedback_module);
  thread transmission_thread(&TransmissionModule::start, &transmission_module);

  control_thread.join();
  feedback_thread.join();
  transmission_thread.join();

  return 0;
}
