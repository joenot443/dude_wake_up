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

struct WaveformOscillator: public Oscillator {
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> frequency;
  
  void tick() override;
  
  WaveformOscillator(std::shared_ptr<Parameter> v) :
  amplitude(std::make_shared<Parameter>("amp", v->paramId, 1.0, 0.0, v->max)),
  shift(std::make_shared<Parameter>("shift", v->paramId, (v->max - v->min) / 2., -3.0, 3.0)),
  frequency(std::make_shared<Parameter>("freq", v->paramId, 0.5, 0.0, 3.0)),
  Oscillator(v) {
    type = Oscillator_waveform;
    parameters = {amplitude, frequency, shift};
    OscillationService::getService()->addOscillator(std::shared_ptr<Oscillator>(this));
  }
};

#endif /* WaveformOscillator_hpp */