//
//  Settings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/2/23.
//

#ifndef Settings_hpp
#define Settings_hpp

#include <stdio.h>
#include "Parameter.hpp"
#include "MidiService.hpp"
#include "Oscillator.hpp"
#include "JSONSerializable.hpp"
#include "json.hpp"
#include <string>

using json = nlohmann::json;

class Settings: public JSONSerializable {
public:
  std::vector<std::shared_ptr<Parameter>> parameters = {};
  
  // Returns all the parameters for the Shader including the ones on the Oscillators
  virtual std::vector<std::shared_ptr<Parameter>> allParameters() {
    std::vector<std::shared_ptr<Parameter>> allParams = {};
    allParams.insert(allParams.end(), parameters.begin(), parameters.end());
    return allParams;
  }

  virtual json serialize() {
    json j;
    
    for (auto p : allParameters()) {
      if (p != nullptr) {
        j[p->name] = p->serialize();
      }
    }
    return j;
  }
  
  std::shared_ptr<Parameter> findParameter(std::string name) {
    for (auto p : allParameters()) {
      if (p != nullptr && p->name == name) {
        return p;
      }
    }
    return NULL;
  }
  
  virtual void load(json j) {
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

#endif /* Settings_hpp */
