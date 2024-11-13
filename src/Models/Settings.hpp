//
//  Settings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/2/23.
//

#ifndef Settings_hpp
#define Settings_hpp

#include <stdio.h>
#include "Console.hpp"
#include "Parameter.hpp"
#include "MidiService.hpp"
#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "JSONSerializable.hpp"
#include "ParameterService.hpp"
#include "json.hpp"
#include "UUID.hpp"
#include <string>

using json = nlohmann::json;

class Settings: public JSONSerializable {
public:
  std::vector<std::shared_ptr<Oscillator>> oscillators;
  std::vector<std::shared_ptr<Parameter>> parameters = {};
  std::string name;
  
  std::string settingsId;
  
  Settings() : name("None"), settingsId(UUID::generateUUID()) {};
  
  Settings(std::string name) : name(name), settingsId(UUID::generateUUID()) {};
  
  virtual void registerParameters() {
    for (auto param : parameters) {
      if (param == nullptr) continue;
      ParameterService::getService()->registerParameter(param);
      param->ownerName = name;
      param->ownerSettingsId = settingsId;
    }
    
    for (auto osc : oscillators) {
      if (osc == nullptr) continue;
      OscillationService::getService()->addOscillator(osc);
    }
  }

  virtual json serialize() {
    json j;
    
    j["param"] = json::object();
    j["osc"] = json::object();
    
    for (auto p : parameters) {
      if (p != nullptr) {
        j["param"][p->name] = p->serialize();
      }
    }
    
    for (auto o : oscillators) {
      if (o != nullptr && o->enabled->boolValue) {
        j["osc"][o->value->name] = o->serialize();
      }
    }
    
    return j;
  }
  
  std::shared_ptr<Parameter> findParameter(std::string name) {
    for (auto p : parameters) {
      if (p != nullptr && p->name == name) {
        return p;
      }
    }
    return NULL;
  }
  
  std::shared_ptr<Oscillator> findOscillator(std::string name) {
    for (auto o : oscillators) {
      if (o != nullptr && o->value->name == name) {
        return o;
      }
    }
    return NULL;
  }
  
  virtual void load(json j) {
    if (!j.is_object()) return;
    
    if (!j["param"].is_object() || !j["osc"].is_object()) {
      log("Invalid JSON format for Settings");
      return;
    }
    
    json paramJ = j["param"];
    json oscJ = j["osc"];
    
    // Iterate through every parameter in the json
    for (auto it = paramJ.begin(); it != paramJ.end(); ++it) {
      // Find the parameter with the name
      auto p = findParameter(it.key());
      if (p != NULL) {
        p->load(it.value());
        // If the parameter has a midiDescriptor, add it to the MidiService
        if (p->midiDescriptor != "") {
          MidiService::getService()->saveAssignment(p, p->midiDescriptor);
        }
        
        
        // // Check if the Oscillator for that Parameter has been saved
        // if (oscJ[p->name].is_object()) {
        //   std::shared_ptr<Oscillator> osc = findOscillator(p->name);
        //   osc->load(oscJ[p->name]);
        //   // Enable the Oscillator
        //   osc->enabled->setBoolValue(true);
        // }
      }
    }
    
    
  }

  // Method to copy parameters and oscillators from another Settings object
  void copyFrom(const Settings& other) {
    // Update each parameter with the corresponding value from the other settings
    for (size_t i = 0; i < parameters.size(); ++i) {
      if (i < other.parameters.size() && parameters[i] && other.parameters[i]) {
        parameters[i]->value = other.parameters[i]->value;
        parameters[i]->intValue = other.parameters[i]->intValue;
        parameters[i]->boolValue = other.parameters[i]->boolValue;
        parameters[i]->midiDescriptor = other.parameters[i]->midiDescriptor;
        parameters[i]->favorited = other.parameters[i]->favorited;
        parameters[i]->ownerName = other.parameters[i]->ownerName;
        parameters[i]->color = other.parameters[i]->color;
      }
    }

//    // Update each oscillator with the corresponding value from the other settings
//    for (size_t i = 0; i < oscillators.size(); ++i) {
//      if (i < other.oscillators.size() && oscillators[i] && other.oscillators[i]) {
//        oscillators[i]->copyFrom(*other.oscillators[i]);
//      }
//    }
  }
};

#endif /* Settings_hpp */
