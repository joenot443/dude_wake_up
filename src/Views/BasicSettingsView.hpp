//
//  BasicSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef BasicSettingsView_hpp
#define BasicSettingsView_hpp

#include <stdio.h>
#include "VideoSettings.h"
#include "OscillatorView.hpp"
#include "ofxImGui.h"

struct BasicSettingsView {
public:
  void setup();
  void update();
  void draw();
  
  VideoSettings *basicSettings;
  
  private:
  void drawHSB();
  void drawBlurSharpen();
  void drawTransform();
  void drawPixelation();
};

#endif /* BasicSettingsView_hpp */
