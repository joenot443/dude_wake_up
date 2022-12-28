//
//  ValueOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-31.
//

#include "ValueOscillator.hpp"
#include "imgui.h"
#include "OscillationService.hpp"
#include "Video.hpp"

ValueOscillator::ValueOscillator(std::shared_ptr<Parameter> o): Oscillator(o) {
  data.reserve(100);
  xRange = {0.0, 10.0};
  yRange = {o->min, o->max};
  observed = o;
}

void ValueOscillator::tick() {
  if (!enabled) return;
  
  float t = frameTime();
  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  this->value = observed->value;
  observed->value = this->value;
  data.push_back(ImVec2(xmod, value));
}
