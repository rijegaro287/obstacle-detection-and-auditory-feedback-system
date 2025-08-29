#include "control_module.hpp"
#include "feedback_module.hpp"
#include "transmission_module.hpp"

#include <thread>

int main() {
  ControlModule& control_module = ControlModule::get_instance();
  FeedbackModule& feedback_module = FeedbackModule::get_instance();
  TransmissionModule& transmission_module = TransmissionModule::get_instance();

  printf("Starting Control, Feedback, and Transmission Modules...\n");
  std::thread control_thread(&ControlModule::start, &control_module);
  std::thread feedback_thread(&FeedbackModule::start, &feedback_module);
  std::thread transmission_thread(&TransmissionModule::start, &transmission_module);

  control_thread.join();
  feedback_thread.join();
  transmission_thread.join();

  return 0;
}
