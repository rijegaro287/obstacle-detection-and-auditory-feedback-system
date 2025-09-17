#pragma once

#include <cstdint>

enum FEEDBACK_MODES {
  NON_VERBAL_MODE,
  VERBAL_MODE
};

class IFeedback{
private:
public:
  static void start_feedback();
  static void stop_feedback();
  static void set_volume(uint64_t volume);
  static void set_feedback_mode(FEEDBACK_MODES mode);
};
