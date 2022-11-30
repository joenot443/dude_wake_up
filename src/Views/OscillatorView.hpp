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
#include "Parameter.hpp"
#include "PulseOscillator.hpp"
#include "ValueOscillator.hpp"

struct OscillatorView {
public:
  void setup();
  void update();
  void draw();
};

#endif /* Waveform_hpp */
