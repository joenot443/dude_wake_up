//
//  OscillationService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#include "OscillationService.hpp"
#include "ConfigService.hpp"
#include "ParameterService.hpp"
#include "Console.hpp"
#include "Strings.hpp"
#include <ostream>

// Service

void OscillationService::addOscillator(std::shared_ptr<Oscillator> obj) {
  oscillators[obj->value->paramId] = obj;
  loadOscillatorSettings(obj);
}

void OscillationService::tickOscillators() {
  for (auto & [key, osc] : oscillators) {
    osc->tick();
  }
}

void OscillationService::loadOscillatorSettings(std::shared_ptr<Oscillator> o) {
  if (oscillatorSettings.count(o->value->name) == 0) {
    return;
  }
  
  auto settingsMap = oscillatorSettings[o->value->name];
  
  for (auto p: o->parameters) {
    if (settingsMap.count(p->name) == 0) continue;
    
    p->setValue(settingsMap[p->name]);
  }
}

void OscillationService::selectOscillator(std::shared_ptr<Oscillator> osc, std::shared_ptr<Parameter> param) {
  // If this is currently our selected oscillator, deselect it
  if (osc == selectedOscillator) {
    selectedOscillator = nullptr;
    return;
  }
  
  selectedOscillator = osc;
  loadOscillatorSettings(osc);
  ParameterService::getService()->selectedParameter = param;
  osc->enabled->boolValue = true;
  ConfigService::getService()->saveDefaultConfigFile();
}

void OscillationService::clear() {
  for (auto & osc : oscillators) {
    osc.second->enabled->boolValue = false;
  }
  oscillators.clear();
  selectedOscillator = nullptr;
}

std::shared_ptr<Oscillator> OscillationService::oscillatorForParameter(std::shared_ptr<Parameter> param) {
  if (oscillators.count(param->paramId) == 0) { return nullptr; }
  return oscillators[param->paramId];
}

// ConfigurableService

json OscillationService::config() {
  json j = json::object();
  
  for (auto & osc : oscillators) {
    j[osc.second->name] = osc.second->parameterValueMap();
  }
  
  return j;
}

void OscillationService::loadConfig(json data) {
  if (data.is_object()) {  
    std::map<std::string, std::map<std::string, float>> itemsMap = data;
    
    
    for (auto const& osc: itemsMap) {
      for (auto const& param: osc.second) {
        oscillatorSettings[osc.first][param.first] = param.second;
      }
    }
  }
}
