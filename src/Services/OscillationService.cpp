//
//  OscillationService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "Console.hpp"
#include "Strings.hpp"
#include <ostream>

// Service

void OscillationService::addOscillator(std::shared_ptr<Oscillator> obj) {
  oscillators.push_front(obj);
  loadOscillatorSettings(obj);
}

void OscillationService::tickOscillators() {
  for (auto & osc : oscillators) {
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
  saveConfigFile();
  ParameterService::getService()->selectedParameter = param;
  osc->enabled->boolValue = true;
}

void OscillationService::saveConfigFile() {
  json j = json::object();

  // Save each oscillator using its name as the key and the serialized data as the value
  for (auto & osc : oscillators) {
    auto paramValueMap = osc->parameterValueMap();
    if (paramValueMap.empty()) continue;
    
    j[osc->name] = paramValueMap;
    oscillatorSettings[osc->name] = paramValueMap;
  }
  
  // Save the json to the config file at ~/oscillator_config.json
  std::ofstream fileStream;
  const char* homeDir = getenv("HOME");
  fileStream.open(formatString("%s/oscillator_config.json", homeDir), ios::trunc);
  if (fileStream.is_open()) {
    fileStream << j.dump(4);
    fileStream.close();
  } else {
    log("Problem saving oscillator_config.");
  }
}

void OscillationService::loadConfigFile() {
  // Load the oscillator config file into a json object
  std::fstream fileStream;
  const char* homeDir = getenv("HOME");
  fileStream.open(formatString("%s/oscillator_config.json", homeDir), ios::in);
  if (fileStream.is_open() && fileStream) {
    json data;
    fileStream >> data;
    
    std::map<std::string, std::map<std::string, float>> itemsMap = data;
    
    
    for (auto const& osc: itemsMap) {
      for (auto const& param: osc.second) {
        oscillatorSettings[osc.first][param.first] = param.second;
      }
    }
    
//    for (const auto& item : j.items()) {
//      std::cout << item.key() << "\n";
//      for (const auto& val : item.value().items()) {
//          oscillatorSettings[item.key()][val.key()] = val.value();
//      }
//    }
  }
}
