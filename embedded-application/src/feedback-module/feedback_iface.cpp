#include "feedback_iface.hpp"
#include "feedback_module.hpp"

void IFeedback::set_feedback_mode(FEEDBACK_MODES mode) {
  FeedbackModule::get_instance().set_feedback_mode(mode);
}
