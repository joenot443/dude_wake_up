//
//  AudioSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioSettingsView_hpp
#define AudioSettingsView_hpp

#include <stdio.h>

#include "VideoSettings.h"
#include "FeedbackSettingsView.hpp"
#include "AudioSettings.h"

struct AudioSettingsView {
public:
  void setup();
  void update();
  void draw();
  void teardown();
  
  AudioAnalysis *analysis;
  std::function<void(int)> closeStream;
    
  AudioSettingsView(AudioAnalysis *analysis, std::function<void(int)> closeStream)
  : analysis(analysis), closeStream(closeStream) {};
  
private:
  void styleWindow();
};

#endif /* AudioSettingsView_hpp */
