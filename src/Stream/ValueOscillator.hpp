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
#include "Parameter.hpp"
#include "Oscillator.hpp"

struct ValueOscillator: public Oscillator {
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> observed = nullptr;
  bool enabled = true;
  float value = 0.0;
  float span = 400.0;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  std::vector<float> values;
  ValueOscillator(std::shared_ptr<Parameter> observed);
  void tick();
};


#endif /* ValueOscillator_hpp */
