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
#include "PulseOscillator.hpp"
#include "Gist.h"

enum AudioSource { AudioSource_none, AudioSource_microphone, AudioSource_file };

struct AudioAnalysisParameter {
  float minSeen = 0.0;
  float maxSeen = 0.0;
  float value = 0.0;
  Parameter *param;
  
  AudioAnalysisParameter(Parameter * param) : param(param) {}
  
  void tick(float val) {
    value = val;
    minSeen = fmin(minSeen, val);
    maxSeen = fmax(maxSeen, val);
    param->value = relationToRange();
  }
  
  float relationToRange() {
    float range = maxSeen - minSeen;
    return value / range;
  }
};

struct AudioAnalysis {
  std::string name;
  bool toggled = false;
  Parameter rms;
  Parameter csd;
  Parameter pitch;
  Parameter energy;
  Parameter zcr;
  Parameter spectralDiff;
  Parameter beat;
  ValueOscillator rmsOscillator;
  ValueOscillator csdOscillator;
  ValueOscillator pitchOscillator;
  ValueOscillator energyOscillator;
  ValueOscillator zcrOscillator;
  ValueOscillator spectralDiffOscillator;
  PulseOscillator beatOscillator;
  AudioAnalysisParameter rmsAnalysisParam;
  AudioAnalysisParameter csdAnalysisParam;
  AudioAnalysisParameter pitchAnalysisParam;
  AudioAnalysisParameter energyAnalysisParam;
  AudioAnalysisParameter zcrAnalysisParam;
  AudioAnalysisParameter spectralDiffAnalysisParam;
  
  std::vector<Parameter *> parameters;
  std::vector<AudioAnalysisParameter *> analysisParameters;
  
  AudioAnalysis(std::string name) :
  name(name),
  rms(Parameter("rms", name, 0.0, 0.0, 1.0)),
  csd(Parameter("csd", name, 0.0, 0.0, 1.0)),
  pitch(Parameter("pitch", name, 0.0, 0.0, 1.0)),
  energy(Parameter("energy", name, 0.0, 0.0, 1.0)),
  zcr(Parameter("zcr", name, 0.0, 0.0, 1.0)),
  spectralDiff(Parameter("spectralDiff", name, 0.0, 0.0, 1.0)),
  beat(Parameter("beat", name, 0.0, 0.0, 1.0)),
  rmsOscillator(ValueOscillator(&rms)),
  pitchOscillator(ValueOscillator(&pitch)),
  csdOscillator(ValueOscillator(&csd)),
  zcrOscillator(ValueOscillator(&zcr)),
  energyOscillator(ValueOscillator(&energy)),
  spectralDiffOscillator(ValueOscillator(&spectralDiff)),
  beatOscillator(PulseOscillator(&beat)),
  parameters({&rms, &csd, &pitch, &energy, &zcr, &beat}),
  rmsAnalysisParam(AudioAnalysisParameter(&rms)),
  csdAnalysisParam(AudioAnalysisParameter(&csd)), 
  pitchAnalysisParam(AudioAnalysisParameter(&pitch)),
  energyAnalysisParam(AudioAnalysisParameter(&energy)),
  zcrAnalysisParam(AudioAnalysisParameter(&zcr)),
  spectralDiffAnalysisParam(AudioAnalysisParameter(&spectralDiff)),
  analysisParameters({&rmsAnalysisParam, &csdAnalysisParam, &pitchAnalysisParam, &energyAnalysisParam, &zcrAnalysisParam, &spectralDiffAnalysisParam})
  {
  };
  
  void analyzeFrame(Gist<float> * gist) {
    rmsAnalysisParam.tick(gist->rootMeanSquare());
    pitchAnalysisParam.tick(gist->pitch());
    csdAnalysisParam.tick(gist->complexSpectralDifference());
    energyAnalysisParam.tick(gist->energyDifference());
    zcrAnalysisParam.tick(gist->zeroCrossingRate());
    spectralDiffAnalysisParam.tick(gist->spectralDifference());
  }
  
};

#endif /* AudioSettings_h */
