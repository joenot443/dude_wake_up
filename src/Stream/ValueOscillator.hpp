//
//  ValueOscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-31.
//

#ifndef ValueOscillator_hpp
#define ValueOscillator_hpp

#include <stdio.h>
#include <vector>
#include "imgui.h"
#include "Parameter.h"

struct ValueOscillator {
  
  Parameter* observed = NULL;
  bool enabled = true;
  float value = 0.0;
  float amplitude = 1.0;
  float shift = 0.0;
  float frequency = 1.0;
  float span = 10.0;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  ImVector<ImVec2> data;
  ValueOscillator(Parameter *observed);
  void tick();
private:
  float frameTime();
};


#endif /* ValueOscillator_hpp */
