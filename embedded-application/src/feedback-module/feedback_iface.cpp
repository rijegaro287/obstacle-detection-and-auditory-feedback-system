#include "feedback_iface.hpp"
#include "feedback_module.hpp"

void IFeedback::start_feedback() {
  FeedbackModule::get_instance().start_feedback();
}

void IFeedback::stop_feedback() {
  FeedbackModule::get_instance().stop_feedback();
}

void IFeedback::set_volume(uint64_t volume) {
  FeedbackModule::get_instance().set_volume(volume);
}

void IFeedback::set_feedback_mode(FEEDBACK_MODES mode) {
  FeedbackModule::get_instance().set_feedback_mode(mode);
}
