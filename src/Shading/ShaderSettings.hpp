//
//  ShaderSettings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/19/22.
//

#ifndef ShaderSettings_h
#define ShaderSettings_h
#include "Parameter.hpp"
#include "MidiService.hpp"
#include "Oscillator.hpp"
#include "VideoSettings.hpp"
#include "JSONSerializable.hpp"
#include "json.hpp"
#include <string>

using json = nlohmann::json;


class ShaderSettings: public JSONSerializable {
  
public:
  std::vector<std::shared_ptr<Parameter>> parameters;
  std::vector<std::shared_ptr<Oscillator>> oscillators;
  
  std::string shaderId;
  
  ShaderSettings(std::string shaderId) :
  shaderId(shaderId)
  {}
  
  ShaderSettings(json j)  {
    shaderId = j["shaderId"];
    load(j);
  }
  
  // Returns all the parameters for the Shader including the ones on the Oscillators
  std::vector<std::shared_ptr<Parameter>> allParameters() {
    std::vector<std::shared_ptr<Parameter>> allParams;
    allParams.insert(allParams.end(), parameters.begin(), parameters.end());
    for (auto osc : oscillators) {
      allParams.insert(allParams.end(), osc->parameters.begin(), osc->parameters.end());
    }
    return allParams;
  }
  
  
  json serialize() {
    json j;
    
    for (auto p : allParameters()) {
      j[p->name] = p->serialize();
    }
    return j;
  }
  
  std::shared_ptr<Parameter> findParameter(std::string name) {
    for (auto p : allParameters()) {
      if (p->name == name) {
        return p;
      }
    }
    return NULL;
  }
  
  void load(json j) {
    if (!j.is_object()) {
      return;
    }
    
    // Iterate through every parameter in the json
    for (auto it = j.begin(); it != j.end(); ++it) {
      // Find the parameter with the name
      auto p = findParameter(it.key());
      if (p != NULL) {
        p->load(it.value());
        // If the parameter has a midiDescriptor, add it to the MidiService
        if (p->midiDescriptor != "") {
          MidiService::getService()->saveAssignment(p, p->midiDescriptor);
        }
      }
    }
  }
};

#endif
