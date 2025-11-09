#pragma once

/**
 * @file feedback_iface.hpp
 * @brief Public façade that exposes the feedback module functionality to the
 * rest of the system while hiding implementation details.
 */

#include "control_iface.hpp"

#include <cstdint>

/**
 * @class IFeedback
 * @ingroup feedback_module
 * @brief Static gateway delegating commands to the @ref FeedbackModule
 * singleton.
 */
class IFeedback{
private:
public:
  /** @brief Request the feedback module to start processing. */
  static void start_feedback();

  /** @brief Request the feedback module to stop processing. */
  static void stop_feedback();

  /**
   * @brief Update the playback volume applied to generated signals.
   * @param volume Percentage in the range [0, 100].
   */
  static void set_volume(uint64_t volume);

  /**
   * @brief Switch between available feedback modes.
   * @param mode Desired feedback modality.
   */
  static void set_feedback_mode(FEEDBACK_MODES mode);
};
