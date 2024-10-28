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
  : WaveformOscillator(param)
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
  float v = 0.0f;

  // Determine the waveform based on the waveShape parameter
  switch (static_cast<WaveShape>(waveShape->intValue)) {
    case Sine:
      v = sin(frequency->value * t + TWO_PI);
      break;
    case Square:
      v = (sin(frequency->value * t + TWO_PI) >= 0) ? 1.0f : -1.0f;
      break;
    case Sawtooth:
      v = 2.0f * (t * frequency->value - floor(t * frequency->value + 0.5f));
      break;
    case Triangle:
      v = 2.0f * fabs(2.0f * (t * frequency->value - floor(t * frequency->value + 0.5f))) - 1.0f;
      break;
  }

  v = OscillatorDampen * amplitude->value * v + shift->value;
  value->setValue(fmin(fmax(v, value->min), value->max));

  // Maintain a fixed number of data points
  if (data.size() >= 1000) {
    data.erase(data.begin()); // Remove the oldest data point
  }
  data.push_back(ImVec2(t, value->value)); // Add the new data point
}

float WaveformOscillator::max()
{
  return amplitude->value + shift->value;
}

float WaveformOscillator::min()
{
  return -amplitude->value + shift->value;
}
