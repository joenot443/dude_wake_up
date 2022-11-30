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
#include "json.hpp"

using json = nlohmann::json;

struct Oscillator {
  std::shared_ptr<Parameter> enabled;
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> frequency;
  
  std::shared_ptr<Parameter> value;

  std::vector<std::shared_ptr<Parameter>> parameters;
  float span;
  
  std::string settingsId;
  std::string name;
  
  std::vector<float> xRange;
  std::vector<float> yRange;
  
  ImVector<ImVec2> data;
  Oscillator(std::shared_ptr<Parameter> v);
  
  void load(json j);
  
  void tick();
  void tick(float value);
  
  json serialize();
  std::map<std::string, float> parameterValueMap();
  
private:
  void pushValue(float value);
};

#endif /* Oscillator_hpp */
