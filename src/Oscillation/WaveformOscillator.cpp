//
//  WaveformOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/26/22.
//

#include "WaveformOscillator.hpp"
#include "Video.hpp"

const float OscillatorDampen = 0.3;

// New constructor implementation
WaveformOscillator::WaveformOscillator(std::shared_ptr<Parameter> param, float amplitude, float shift, float frequency)
  : Oscillator(param)
{
  this->amplitude = std::make_shared<Parameter>("amplitude", amplitude, 0.0, 1.0);
  this->shift = std::make_shared<Parameter>("shift", shift, -1.0, 1.0);
  this->frequency = std::make_shared<Parameter>("frequency", frequency, 0.0, 10.0);
}

void WaveformOscillator::tick()
{
  if (!enabled->boolValue)
    return;

  float t = frameTime();
  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
    data.shrink(0);

  float v = OscillatorDampen * amplitude->value * sin(frequency->value * t + TWO_PI) + shift->value;
  value->setValue(fmin(fmax(v, value->min), value->max));
  data.push_back(ImVec2(xmod, value->value));
}

float WaveformOscillator::max()
{
  return amplitude->value + shift->value;
}

float WaveformOscillator::min()
{
  return -amplitude->value + shift->value;
}
