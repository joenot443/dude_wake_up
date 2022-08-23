//
//  Waveform.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef Waveform_hpp
#define Waveform_hpp

#include <stdio.h>
#include "Oscillator.hpp"
#include "Parameter.h"
#include "PulseOscillator.hpp"
#include "ValueOscillator.hpp"

struct OscillatorView {
  static void draw(const char* name, Parameter* value, PulseOscillator *oscillator);
  static void draw(const char* name, Parameter* value, ValueOscillator *oscillator);
  static void draw(const char* name, Parameter* value, Oscillator *oscillator);
};

#endif /* Waveform_hpp */
