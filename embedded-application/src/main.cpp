#include "control_module.hpp"
#include "feedback_module.hpp"
#include "transmission_module.hpp"

#include <thread>

int main() {
  ControlModule& control_module = ControlModule::get_instance();
  FeedbackModule& feedback_module = FeedbackModule::get_instance();

  printf("Starting Control and Feedback Modules...\n");

  // control_module.start();
  // feedback_module.start();

  std::thread control_thread(&ControlModule::start, &control_module);
  std::thread feedback_thread(&FeedbackModule::start, &feedback_module);

  control_thread.join();
  feedback_thread.join();

  return 0;
}
