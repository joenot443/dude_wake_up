//
//  AudioSourceBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/23/22.
//

#ifndef AudioSourceBrowserView_hpp
#define AudioSourceBrowserView_hpp

#include "AudioSource.hpp"
#include "ofMain.h"
#include "Parameter.hpp"
#include <stdio.h>

struct AudioSourceBrowserView {
public:
  void setup();
  void update();
  void draw();
  void keyReleased(int key);
  
  std::shared_ptr<Parameter> samplePlaybackSliderPosition;
  
  AudioSourceBrowserView() : samplePlaybackSliderPosition(std::make_shared<Parameter>("Position", 0.0, 0.0, 1.0)) {}
  
private:
  void updatePlaybackPosition();
  void drawAudioSourceSelector();
  void drawSelectedAudioSource();
  void drawStartAnalysisButton();
  void drawSampleTrack();
};

#endif /* AudioSourceBrowserView_hpp */
