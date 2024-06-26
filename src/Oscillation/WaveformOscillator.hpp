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

struct WaveformOscillator: public Oscillator {
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> frequency;
  
  void tick() override;
  
  float max();
  float min();
  
  float randOffset = ofRandom(TWO_PI);
  
  WaveformOscillator(std::shared_ptr<Parameter> v) :
  amplitude(std::make_shared<Parameter>("amp", 1.0, 0.0, v->max * 2.0, ParameterType_Hidden)),
  shift(std::make_shared<Parameter>("shift", (v->max - abs(v->min)) / 2., -3.0, 3.0, ParameterType_Hidden)),
  frequency(std::make_shared<Parameter>("freq", 0.5, 0.0, 3.0, ParameterType_Hidden)),
  Oscillator(v) {
    type = Oscillator_waveform;
    parameters = {amplitude, frequency, shift};
  }
};

#endif /* WaveformOscillator_hpp */
