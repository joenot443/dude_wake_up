//
//  VideoRecordingView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/25/24.
//

#ifndef VideoRecordingView_hpp
#define VideoRecordingView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoRecorder.hpp"

class VideoRecordingView {
  VideoRecordingSettings settings;
  
  void setup();
  void update();
  void draw();
};

#endif /* VideoRecordingView_hpp */
