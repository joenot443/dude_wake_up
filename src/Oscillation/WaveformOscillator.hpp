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
  Triangle
};

struct WaveformOscillator: public Oscillator {
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> frequency;
  std::shared_ptr<Parameter> waveShape;
  
  void tick() override;
  
  float max();
  float min();
  
  float randOffset = ofRandom(TWO_PI);
  
  WaveformOscillator(std::shared_ptr<Parameter> v) :
  amplitude(std::make_shared<Parameter>("amp", (v->min+v->max) / 2.0, 0.0, v->max * 2.0, ParameterType_Hidden)),
  shift(std::make_shared<Parameter>("shift", v->defaultValue + (v->max - v->min) / 2., v->min * -3.0, v->max * 3.0, ParameterType_Hidden)),
  frequency(std::make_shared<Parameter>("freq", 0.5, 0.0, 3.0, ParameterType_Hidden)),
  waveShape(std::make_shared<Parameter>("Wave Shape", Sine, 0, 3)),
  Oscillator(v) {
    parameters = {amplitude, frequency, shift};
  }
  
  WaveformOscillator(std::shared_ptr<Parameter> param, float amplitude, float shift, float frequency);
  
  OscillatorType type() override {
    return OscillatorType_waveform;
  }
};

#endif /* WaveformOscillator_hpp */
