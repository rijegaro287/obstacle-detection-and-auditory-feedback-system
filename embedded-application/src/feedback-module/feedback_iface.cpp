/**
 * @file feedback_iface.cpp
 * @brief Implements the @ref IFeedback façade.
 */

#include "feedback_iface.hpp"
#include "feedback_module.hpp"

/** See @ref IFeedback::start_feedback. */
void IFeedback::start_feedback() {
  FeedbackModule::get_instance().start_feedback();
}

/** See @ref IFeedback::stop_feedback. */
void IFeedback::stop_feedback() {
  FeedbackModule::get_instance().stop_feedback();
}

/** See @ref IFeedback::set_volume. */
void IFeedback::set_volume(uint64_t volume) {
  FeedbackModule::get_instance().set_volume(volume);
}

/** See @ref IFeedback::set_feedback_mode. */
void IFeedback::set_feedback_mode(FEEDBACK_MODES mode) {
  FeedbackModule::get_instance().set_feedback_mode(mode);
}
