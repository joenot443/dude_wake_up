//
//  Waveform.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef Waveform_hpp
#define Waveform_hpp

#include "Oscillator.hpp"
#include "Parameter.hpp"
#include "PulseOscillator.hpp"
#include "ValueOscillator.hpp"
#include <stdio.h>

struct OscillatorView {
public:
  static void draw(std::shared_ptr<Oscillator> oscillator,
                   std::shared_ptr<Parameter> value);
};

#endif /* Waveform_hpp */
