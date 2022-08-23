//
//  OscillationService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#include "OscillationService.hpp"

// Service

void OscillationService::addOscillator(Oscillator * obj) {
  oscillators.push_front(obj);
}

void OscillationService::tickOscillators() {
  for (auto & osc : oscillators) {
    osc->tick();
  }
  for (auto & osc : valueOscillators) {
    osc->tick();
  }
  for (auto & osc : pulseOscillators) {
    osc->tick();
  }
}

void OscillationService::addValueOscillator(ValueOscillator * obj) {
  valueOscillators.push_front(obj);
}

void OscillationService::addPulseOscillator(PulseOscillator * obj) {
  pulseOscillators.push_front(obj);
}
