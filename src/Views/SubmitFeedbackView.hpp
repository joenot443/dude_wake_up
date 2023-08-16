#include "ofMain.h"
#include "CommonViews.hpp"
#include "Feedback.hpp"

#ifndef SubmitFeedbackView_hpp
#define SubmitFeedbackView_hpp

struct SubmitFeedbackView
{
  Feedback feedback;
  void setup();
  void update();
  void draw();
  void submitFeedback();

  static const char *popupId;
};

#endif /* SubmitFeedbackView_hpp */
