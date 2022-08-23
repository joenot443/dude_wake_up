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

ValueOscillator::ValueOscillator(Parameter *val) {
  data.reserve(100);
  xRange = {0.0, 10.0};
  yRange = {val->min, val->max};
  observed = val;
  OscillationService::getService()->addValueOscillator(this);
}

void ValueOscillator::tick() {
  if (!enabled) return;
  
  float t = frameTime();
  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  this->value = observed->value * amplitude + shift;
  observed->value = this->value;
  data.push_back(ImVec2(xmod, value));
}
