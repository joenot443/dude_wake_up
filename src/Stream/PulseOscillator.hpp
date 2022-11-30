//
//  PulseOscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/6/22.
//

#ifndef PulseOscillator_hpp
#define PulseOscillator_hpp

#include <stdio.h>
#include <chrono>
#include "Parameter.hpp"
#include "imgui.h"

struct PulseOscillator {
  bool enabled = false;
  bool pulsing = false;

  std::shared_ptr<Parameter> value = nullptr;
  float amplitude = 1.0;
  float time = 0.0;
  std::chrono::time_point<std::chrono::system_clock> lastBeatTimestamp;
  std::chrono::duration<double> beatLength;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  ImVector<ImVec2> data;
  PulseOscillator(std::shared_ptr<Parameter> o);
  
  void tick();
  void pulse();
private:
  void pushValue(float value);
};


#endif /* PulseOscillator_hpp */
