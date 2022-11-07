//
//  ShaderSettings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/19/22.
//

#ifndef ShaderSettings_h
#define ShaderSettings_h
#include "Parameter.hpp"
#include "Oscillator.hpp"
#include "VideoSettings.hpp"
#include "JSONSerializable.hpp"
#include "json.hpp"
#include <string>

using json = nlohmann::json;


class ShaderSettings: public JSONSerializable {

public:
  std::vector<Parameter*> parameters;
  std::vector<Oscillator*> oscillators;
  
  std::string shaderId;
  
  
  ShaderSettings(std::string shaderId) :
  shaderId(shaderId)
  {}
  
  ShaderSettings(json j)  {
    shaderId = j["shaderId"];
    load(j);
  }
  

  
  json serialize() {
    json j;
    
    for (auto p : parameters) {
      j[p->paramId] = p->paramValue();
    }
    auto ops = Oscillator::parametersFromOscillators(oscillators);
    for (auto p : ops) {
      j[p->paramId] = p->paramValue();
    }
    return j;
  }
  
  void load(json j) {
    if (!j.is_object()) {
      return;
    }

    for (auto p : parameters) {
      // Check if the parameter is in the json
      if (j.find(p->paramId) != j.end()) {
        p->setValue(j[p->paramId]);
      }
    }
    for (auto o : oscillators) {
      o->frequency.setValue(j[o->frequency.paramId.c_str()]);
      o->amplitude.setValue(j[o->amplitude.paramId.c_str()]);
      o->shift.setValue(j[o->shift.paramId.c_str()]);
      o->enabled.setValue(j[o->enabled.paramId.c_str()]);
    }
  }
};

#endif
