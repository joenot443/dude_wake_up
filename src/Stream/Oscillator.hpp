//
//  Oscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#ifndef Oscillator_hpp
#define Oscillator_hpp

#include <stdio.h>
#include "Parameter.h"
#include "imgui.h"


struct Oscillator {
  bool enabled = false;

  Parameter *value;
  float amplitude;
  float shift;
  float frequency;
  float span;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  ImVector<ImVec2> data;
  Oscillator(Parameter *);
  
  void tick();
  void tick(float value);
private:
  void pushValue(float value);
  float frameTime();
};

#endif /* Oscillator_hpp */
