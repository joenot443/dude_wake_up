//
//  VideoSourceBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#ifndef VideoSourceBrowserView_hpp
#define VideoSourceBrowserView_hpp

#include "VideoSource.hpp"
#include "ofMain.h"
#include <stdio.h>

struct VideoSourceBrowserView {
public:
  void setup();
  void update();
  void draw();
  std::shared_ptr<VideoSource> selectedVideoSource;
  
private:
  void drawVideoSourceSelector();
  void drawVideoSourceFrame();

  ofFbo videoSourceFbo;
};

#endif /* VideoSourceBrowserView_hpp */
