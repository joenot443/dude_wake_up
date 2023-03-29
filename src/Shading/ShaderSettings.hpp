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
#include "JSONSerializable.hpp"
#include "json.hpp"
#include "Settings.hpp"
#include <string>

using json = nlohmann::json;

class ShaderSettings: public Settings {
  
public:
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
      if (osc != nullptr) {
        allParams.insert(allParams.end(), osc->parameters.begin(), osc->parameters.end());
      }
    }
    return allParams;
  }
  
};

#endif
