//
//  AudioSettings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioSettings_h
#define AudioSettings_h

#include "Oscillator.hpp"
#include "Parameter.h"
#include "ValueOscillator.hpp"

enum AudioSource { AudioSource_none, AudioSource_microphone, AudioSource_file };

struct AudioAnalysis {
  std::string name;
  bool toggled = false;
  Parameter rms;
  Parameter csd;
  Parameter pitch;
  Parameter energy;
  Parameter zcr;
  Parameter spectralDiff;
  ValueOscillator rmsOscillator;
  ValueOscillator csdOscillator;
  ValueOscillator pitchOscillator;
  ValueOscillator energyOscillator;
  ValueOscillator zcrOscillator;
  ValueOscillator spectralDiffOscillator;
  
  std::vector<Parameter *> parameters;
  AudioAnalysis(std::string name) :
  name(name),
  rms(Parameter("rms", name, 0.0)),
  csd(Parameter("csd", name, 0.0)),
  pitch(Parameter("pitch", name, 0.0)),
  energy(Parameter("energy", name, 0.0)),
  zcr(Parameter("zcr", name, 0.0)),
  spectralDiff(Parameter("spectralDiff", name, 0.0)),
  rmsOscillator(ValueOscillator(&rms)),
  pitchOscillator(ValueOscillator(&pitch)),
  csdOscillator(ValueOscillator(&csd)),
  zcrOscillator(ValueOscillator(&zcr)),
  energyOscillator(ValueOscillator(&energy)),
  spectralDiffOscillator(ValueOscillator(&spectralDiff)),
  parameters({&rms, &csd, &pitch, &energy, &zcr})
  {
  }
};

#endif /* AudioSettings_h */
