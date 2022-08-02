//
//  Oscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#include "Oscillator.hpp"
#include "OscillationService.hpp"

Oscillator::Oscillator(Parameter *val) {
  value = val;
  data = ImVector<ImVec2>();
  data.reserve(100);
  xRange = {0.0, 10.0};
  yRange = {val->min, val->max};
  amplitude = 1.0;
  shift = 0.0;
  frequency = 1.0;
  span = 10.0;
  OscillationService::getService()->addOscillator(this);
}

float Oscillator::frameTime() {
  static float t = 0.0;
  t += ImGui::GetIO().DeltaTime;
  return t;
}

void Oscillator::tick() {
  if (!enabled) return;
  
  float t = frameTime();
  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  value->value = amplitude * sin(frequency * t) + shift;
  data.push_back(ImVec2(xmod, value->value));
}
