//
//  AudioStream.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioStream_hpp
#define AudioStream_hpp

#include <stdio.h>
#include "ofSoundStream.h"
#include "ofMain.h"
#include "AudioSettings.hpp"
#include "MainSettings.hpp"
//#include "Gist.h"
//#include "BTrack.h"
#include "PulseOscillator.hpp"

const static int GFrameSize = 512;
const static int GSampleRate = 44100;

class AudioStream {
public:
  AudioStream(AudioStreamConfig config) :
  config(config),
  beatPulse(std::make_shared<Parameter>("beat_pulse", "beat_pulse_0", 0.0, 0.0, 1.0)),
  pulseOscillator(std::make_shared<PulseOscillator>(beatPulse))
  {};
  void setup();
  void update();
  bool isSetup;
  AudioAnalysis *analysis;
private:
  AudioStreamConfig config;
  ofSoundStream stream;
//  Gist<float> gist = Gist<float>(GFrameSize, GSampleRate);
//  BTrack b;
  std::shared_ptr<PulseOscillator> pulseOscillator;
  std::shared_ptr<Parameter> beatPulse;

  std::vector<float> buffer;
  
  // ofSoundStream Callbacks
  void audioIn(ofSoundBuffer &soundBuffer);
};

#endif /* AudioStream_hpp */
