//
//  WaveformOscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/26/22.
//

#ifndef WaveformOscillator_hpp
#define WaveformOscillator_hpp

#include <stdio.h>
#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "ofMain.h"

enum WaveShape {
  Sine,
  Square,
  Sawtooth,
  Triangle,
  PulseTrain,
  ExponentialSine,
  HarmonicStack,
  RectifiedSine,
  NoiseModulatedSine,
  BitcrushedSine,
  MoireOscillation
};

struct WaveformOscillator: public Oscillator {
  std::shared_ptr<Parameter> minOutput;
  std::shared_ptr<Parameter> maxOutput;
  std::shared_ptr<Parameter> frequency;
  std::shared_ptr<Parameter> waveShape;
  bool showMinMax = true;

  void tick() override;

  float max();
  float min();

  float randOffset = ofRandom(TWO_PI);

  WaveformOscillator(std::shared_ptr<Parameter> v) :
  minOutput(std::make_shared<Parameter>("Min Output",
    v->min + (v->max - v->min) * 0.3,  // Default: lower 30% of range
    v->min,
    v->max,
    ParameterType_Hidden)),
  maxOutput(std::make_shared<Parameter>("Max Output",
    v->min + (v->max - v->min) * 0.7,  // Default: upper 70% of range
    v->min,
    v->max,
    ParameterType_Hidden)),
  frequency(std::make_shared<Parameter>("Frequency", 1.0, 0.0, 10.0, ParameterType_Hidden)),
  waveShape(std::make_shared<Parameter>("Wave Shape", Sine, 0, 10)),
  Oscillator(v) {
    parameters = {minOutput, maxOutput, frequency};
  }
  
  WaveformOscillator(std::shared_ptr<Parameter> param, float amplitude, float shift, float frequency);
  
  OscillatorType type() override {
    return OscillatorType_waveform;
  }
};

#endif /* WaveformOscillator_hpp */
