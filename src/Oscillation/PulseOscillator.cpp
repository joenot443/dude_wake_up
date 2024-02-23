//
//  PulseOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/6/22.
//

#include <cmath>
#include "PulseOscillator.hpp"
#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "Console.hpp"
#include "Video.hpp"

PulseOscillator::PulseOscillator(std::shared_ptr<Parameter> o) : Oscillator(o), value(o) {
  data.reserve(PulseOscillatorValueCount);
  type = Oscillator_pulse;
  time = 0.0;
  xRange = {0.0, static_cast<float>(PulseOscillatorValueCount)};
  yRange = {o->min, o->max};
  lastBeatTimestamp = std::chrono::system_clock::now();
}

void PulseOscillator::tick() {
  static float t = 0.;
  if (enabled) {
    t += 1.;
    data.push_back(ImVec2(t, value->value));
    if (data.size() > PulseOscillatorValueCount) {
      t = 0.;
      data.clear();
    }
  }
}

void PulseOscillator::pulse() {
  log("THATS A BEAT SON");
  beatLength = std::chrono::system_clock::now() - lastBeatTimestamp;
  log("Last beat was %lf", beatLength.count());
  lastBeatTimestamp = std::chrono::system_clock::now();
  this->value->value = yRange[0];
  pulsing = true;
  time = 0.0;
}
