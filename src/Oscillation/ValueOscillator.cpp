//
//  ValueOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-31.
//

#include "ValueOscillator.hpp"
#include "Oscillation/Oscillator.hpp"
#include "imgui.h"
#include "OscillationService.hpp"
#include "Video.hpp"

ValueOscillator::ValueOscillator(std::shared_ptr<Parameter> o): Oscillator(o) {
  values.reserve(span);
  xRange = {0.0, span};
  yRange = {o->min, o->max};
  observed = o;
}

void ValueOscillator::tick() {
  if (!enabled) return;
  
  if (values.size() >= span)
    values.clear();
  
  this->value = observed->value;
  observed->value = this->value;
  values.push_back(value);
}
