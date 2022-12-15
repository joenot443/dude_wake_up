//
//  OutputWindow.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#ifndef OutputWindow_hpp
#define OutputWindow_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"
#include "ofxImGui.h"


struct OutputWindow : public ofBaseApp {
  std::shared_ptr<VideoSource> videoSource;

  OutputWindow(std::shared_ptr<VideoSource> videoSource) : videoSource(videoSource) {};

  ofFbo outputFbo;
  void setup();
  void update();
  void draw();
  void teardown();
};

#endif /* OutputWindow_hpp */
