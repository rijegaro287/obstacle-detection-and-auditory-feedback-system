#pragma once

enum FEEDBACK_MODES {
  NON_VERBAL_MODE,
  VERBAL_MODE
};

class IFeedback{
private:
public:
  static void set_feedback_mode(FEEDBACK_MODES mode);
};