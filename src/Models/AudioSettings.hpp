//
//  AudioSettings.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-30.
//

#ifndef AudioSettings_h
#define AudioSettings_h

#include "Oscillator.hpp"
#include "Parameter.hpp"
#include "ValueOscillator.hpp"
#include "PulseOscillator.hpp"
#include "Gist.h"

struct AudioAnalysisParameter {
  float minSeen = 0.0;
  float maxSeen = 0.0;
  float value = 0.0;
  std::shared_ptr<Parameter> param;
  
  AudioAnalysisParameter(std::shared_ptr<Parameter> param) : param(param) {}
  
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
  std::shared_ptr<Parameter> rms;
  std::shared_ptr<Parameter> csd;
  std::shared_ptr<Parameter> pitch;
  std::shared_ptr<Parameter> energy;
  std::shared_ptr<Parameter> zcr;
  std::shared_ptr<Parameter> spectralDiff;
  std::shared_ptr<Parameter> beat;
  
  std::shared_ptr<ValueOscillator> rmsOscillator;
  std::shared_ptr<ValueOscillator> csdOscillator;
  std::shared_ptr<ValueOscillator> pitchOscillator;
  std::shared_ptr<ValueOscillator> energyOscillator;
  std::shared_ptr<ValueOscillator> zcrOscillator;
  std::shared_ptr<ValueOscillator> spectralDiffOscillator;
  std::shared_ptr<PulseOscillator> beatOscillator;
  AudioAnalysisParameter rmsAnalysisParam;
  AudioAnalysisParameter csdAnalysisParam;
  AudioAnalysisParameter pitchAnalysisParam;
  AudioAnalysisParameter energyAnalysisParam;
  AudioAnalysisParameter zcrAnalysisParam;
  AudioAnalysisParameter spectralDiffAnalysisParam;
  
  std::vector<std::shared_ptr<Parameter>> parameters;
  std::vector<AudioAnalysisParameter *> analysisParameters;
  
  AudioAnalysis(std::string name) :
  name(name),
  rms(std::make_shared<Parameter>("rms", name, 0.0, 0.0, 1.0)),
  csd(std::make_shared<Parameter>("csd", name, 0.0, 0.0, 1.0)),
  pitch(std::make_shared<Parameter>("pitch", name, 0.0, 0.0, 1.0)),
  energy(std::make_shared<Parameter>("energy", name, 0.0, 0.0, 1.0)),
  zcr(std::make_shared<Parameter>("zcr", name, 0.0, 0.0, 1.0)),
  spectralDiff(std::make_shared<Parameter>("spectralDiff", name, 0.0, 0.0, 1.0)),
  beat(std::make_shared<Parameter>("beat", name, 0.0, 0.0, 1.0)),
  rmsOscillator(std::make_shared<ValueOscillator>(rms)),
  pitchOscillator(std::make_shared<ValueOscillator>(pitch)),
  csdOscillator(std::make_shared<ValueOscillator>(csd)),
  zcrOscillator(std::make_shared<ValueOscillator>(zcr)),
  energyOscillator(std::make_shared<ValueOscillator>(energy)),
  spectralDiffOscillator(std::make_shared<ValueOscillator>(spectralDiff)),
  beatOscillator(std::make_shared<PulseOscillator>(beat)),
  parameters({rms, csd, pitch, energy, zcr, beat}),
  rmsAnalysisParam(AudioAnalysisParameter(rms)),
  csdAnalysisParam(AudioAnalysisParameter(csd)), 
  pitchAnalysisParam(AudioAnalysisParameter(pitch)),
  energyAnalysisParam(AudioAnalysisParameter(energy)),
  zcrAnalysisParam(AudioAnalysisParameter(zcr)),
  spectralDiffAnalysisParam(AudioAnalysisParameter(spectralDiff)),
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
