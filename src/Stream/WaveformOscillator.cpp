//
//  WaveformOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/26/22.
//

#include "WaveformOscillator.hpp"
#include "Video.hpp"

const float OscillatorDampen = 0.3;

void WaveformOscillator::tick()  {
  if (!enabled->boolValue) return;
  
  float t = frameTime();
  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  float v = OscillatorDampen * amplitude->value * sin(frequency->value * t) + shift->value;
  value->setValue(fmin(fmax(v, value->min), value->max));
  data.push_back(ImVec2(xmod, value->value));
}

