//
//  PulseOscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/6/22.
//

#include "PulseOscillator.hpp"
#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "Console.hpp"
#include "Video.hpp"

PulseOscillator::PulseOscillator(Parameter *param) : value(param) {
  time = 0.0;
  xRange = {0.0, 10.0};
  yRange = {param->min, param->max};
  OscillationService::getService()->addPulseOscillator(this);
  lastBeatTimestamp = std::chrono::system_clock::now();
}

void PulseOscillator::tick() {
  if (enabled) {
    float y = yRange[0];
    time += (frameTime() / 1000.0);
    if (pulsing) {
      // y = - a * (x - 1)^2 + 1      
      y = - amplitude * pow((2*time - 1.0), 2) + 1.0;

      // We've finished the pulse
      if (y < yRange[0]) {
        log("Pulse done");
        pulsing = false;
        return;
      }
    }
    this->value->value = y;
    float xmod = fmodf(time, xRange[1]);
    if (!data.empty() && xmod < data.back().x)
        data.shrink(0);
    data.push_back(ImVec2(xmod, y));
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
