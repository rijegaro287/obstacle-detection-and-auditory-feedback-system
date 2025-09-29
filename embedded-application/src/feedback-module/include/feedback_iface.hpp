#pragma once

#include "control_iface.hpp"

#include <cstdint>

class IFeedback{
private:
public:
  static void start_feedback();
  static void stop_feedback();
  static void set_volume(uint64_t volume);
  static void set_feedback_mode(FEEDBACK_MODES mode);
};
