//
//  AudioSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/22.
//

#ifndef AudioSource_h
#define AudioSource_h

#include "json.hpp"
#include "ofMain.h"
#include "Gist.h"
#include "AudioSettings.hpp"

using json = nlohmann::json;

struct AudioSource {
  std::string id;
  std::string deviceId;
  std::string name;
  bool active = false;
  ofSoundDevice device;
  ofSoundStream stream;
  AudioAnalysis audioAnalysis;
  
  Gist<float> gist = Gist<float>(512, 44100);
  
  json serialize() { return 0; };
  bool enabled = false;
  
  void load(json j){};
  
  void setup();
  void toggle();
  void disable();
  
  void update();
  
  void audioIn(ofSoundBuffer &soundBuffer);
  
  void processFrame(std::vector<float> frame);
  
  void debugGist();
};

#endif /* AudioSource_h */
