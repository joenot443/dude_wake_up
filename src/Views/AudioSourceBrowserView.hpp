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

  enum FrequencyViewMode {
    FrequencyViewMode_Bands = 0,
    FrequencyViewMode_Bars = 1,
    FrequencyViewMode_Waveform = 2
  };

  AudioSourceBrowserView() :
    samplePlaybackSliderPosition(std::make_shared<Parameter>("Position", 0.0, 0.0, 1.0)),
    frequencyViewMode(FrequencyViewMode_Bands) {}

private:
  FrequencyViewMode frequencyViewMode;
  void updatePlaybackPosition();
  void drawAudioSourceSelector();
  void drawSelectedAudioSource();
  void drawStartAnalysisButton();
  void drawSampleTrack();
};

#endif /* AudioSourceBrowserView_hpp */
