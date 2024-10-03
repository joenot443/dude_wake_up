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

enum AudioSourceType {
  AudioSourceType_Microphone,
  AudioSourceType_File
};

class AudioSource {
public:
  std::string id;
  std::string name;
  bool active = false;
  AudioAnalysis audioAnalysis;
  Gist<float> gist = Gist<float>(512, 44100);
  
  virtual ~AudioSource() = default;
  
  virtual json serialize() { return json(); }
  
  virtual void load(json j) {}
  
  virtual AudioSourceType type() = 0;
  
  virtual void setup() = 0;
  virtual void toggle();
  virtual void disable() = 0;
  virtual void audioIn(ofSoundBuffer &soundBuffer);
  virtual void processFrame(const std::vector<float>& frame);
  virtual void debugGist();
};

#endif /* AudioSource_h */
