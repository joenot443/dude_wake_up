//
//  Oscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "Video.hpp"

Oscillator::Oscillator(Parameter *v) :
value(v),
settingsId(value->paramId),
amplitude(Parameter("amp", value->paramId, 1.0, 0.0, value->max)),
shift(Parameter("shift", value->paramId, 0.0, -3.0, 3.0)),
frequency(Parameter("freq", value->paramId, 1.0, -3.0, 3.0)),
enabled(Parameter("enabled", v->paramId, 0.0, 0.0, 1.0))
{
  parameters = {&amplitude, &frequency, &shift, &enabled};
  data = ImVector<ImVec2>();
  data.reserve(100);
  xRange = {0.0, 10.0};
  span = 10.0;
  yRange = {value->min, value->max};
  OscillationService::getService()->addOscillator(this);
}

void Oscillator::tick() {
  if (!enabled.boolValue) return;
  
  float t = frameTime();  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  float v = amplitude.value * sin(frequency.value * t) + shift.value;
  value->setValue(fmin(fmax(v, value->min), value->max));
  data.push_back(ImVec2(xmod, value->value));
}
