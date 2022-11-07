//
//  Oscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#ifndef Oscillator_hpp
#define Oscillator_hpp

#include <stdio.h>
#include "Parameter.hpp"
#include "imgui.h"


struct Oscillator {
  
  Parameter enabled;
  Parameter *value;
  Parameter amplitude;
  Parameter shift;
  Parameter frequency;
  std::vector<Parameter *> parameters;
  float span;
  
  std::string settingsId;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  ImVector<ImVec2> data;
  Oscillator(Parameter *);
  
  
  void tick();
  void tick(float value);
  
  static std::vector<Parameter*> parametersFromOscillators(std::vector<Oscillator*> oscs) {
    std::vector<Parameter*> ret = {};
    
    for (auto o: oscs) {
      auto p = o->parameters;
      ret.insert( ret.end(), p.begin(), p.end() );
    }
    
    return ret;
  }

private:
  void pushValue(float value);
};




#endif /* Oscillator_hpp */
