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
#include "ParameterService.hpp"
#include "Oscillator.hpp"
#include "JSONSerializable.hpp"
#include "json.hpp"
#include "Settings.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;

class ShaderSettings: public Settings {
  
public:  
  std::string shaderId;
  
  // Layout
  std::shared_ptr<Parameter> x;
  std::shared_ptr<Parameter> y;
  
  // Audio
  std::shared_ptr<Parameter> audioReactiveParameter;
  
  ShaderSettings(std::string shaderId, json j, std::string name) :
  Settings(name),
  shaderId(shaderId),
  x(std::make_shared<Parameter>("x", 0.0, 0.0, 10000.0)),
  y(std::make_shared<Parameter>("y", 0.0, 0.0, 10000.0))
  {
    x->hiddenFromNode = true;
    y->hiddenFromNode = true;
    load(j);
    registerParameters();
  }
  
  // Returns all the parameters for the Shader including the ones on the Oscillators
  std::vector<std::shared_ptr<Parameter>> allParameters() {
    std::vector<std::shared_ptr<Parameter>> allParams;
    allParams.push_back(x);
    allParams.push_back(y);
    allParams.insert(allParams.end(), parameters.begin(), parameters.end());
    
    for (auto osc : oscillators) {
      if (osc != nullptr) {
        allParams.insert(allParams.end(), osc->parameters.begin(), osc->parameters.end());
      }
    }
    // Remove any nullptrs which may have been amended
    allParams.erase(std::remove(allParams.begin(), allParams.end(), nullptr), allParams.end());
    
    return allParams;
  }
};

#endif
