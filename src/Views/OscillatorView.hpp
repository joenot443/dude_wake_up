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
#include "WaveformOscillator.hpp"
#include "ValueOscillator.hpp"
#include <stdio.h>

typedef std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>> OscillatorParam;

static const ImVec2 ChartSize = ImVec2(190, 190);

struct OscillatorView {
public:
  static void draw(std::shared_ptr<Oscillator> oscillator,
                   std::shared_ptr<Parameter> value,
                   ImVec2 size = ChartSize,
                   bool drawExtras = true);

  static void draw(std::vector<std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>> subjects,
                   ImVec2 size = ChartSize,
                   bool drawExtras = true);
  
  static void drawMini(std::shared_ptr<Oscillator> oscillator,
                       std::shared_ptr<Parameter> value,
                       bool drawExtras = true);
};

#endif /* Waveform_hpp */
