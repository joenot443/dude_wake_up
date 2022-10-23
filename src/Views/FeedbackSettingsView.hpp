//
//  FeedbackSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-13.
//

#ifndef FeedbackSettingsView_hpp
#define FeedbackSettingsView_hpp

#include <stdio.h>
#include "VideoSettings.hpp"

struct FeedbackSettingsView {
public:
  void setup();
  void update();
  void draw();
  
  FeedbackSettings *feedbackSettings;
  
  private:
  void drawParameters();
  void drawMisc();
  void drawRanges();
  void drawHSB();
};


#endif /* FeedbackSettingsView_hpp */
