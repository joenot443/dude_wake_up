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
#include <stdio.h>

struct AudioSourceBrowserView {
public:
  void setup();
  void update();
  void draw();

private:
  void drawAudioSourceSelector();
  void drawSelectedAudioSource();
};

#endif /* AudioSourceBrowserView_hpp */