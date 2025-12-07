//
//  OscillationService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#include "OscillationService.hpp"
#include "WaveformOscillator.hpp"
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
    if (osc == nullptr) continue;
    if (osc->enabled->boolValue) {
      osc->tick();
    }
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

  for (auto & [id, osc] : oscillators) {
    if (osc->enabled->boolValue && osc->type() == OscillatorType_waveform) {
      std::shared_ptr<WaveformOscillator> wOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);
      json oscJ = json::object();
      oscJ["freq"] = wOsc->frequency->value;
      oscJ["minOutput"] = wOsc->minOutput->value;
      oscJ["maxOutput"] = wOsc->maxOutput->value;
      j[wOsc->value->paramId] = oscJ;
    }
  }

  return j;
}

void OscillationService::loadConfig(json data) {
  if (data.is_object()) {
    std::map<std::string, std::map<std::string, float>> itemsMap = data;

    for (auto & [id, values] : itemsMap) {
      std::string paramId = id;
      if (oscillators.count(paramId) != 0) {
        std::shared_ptr<Oscillator> osc = oscillators[id];

        if (osc->type() == OscillatorType_waveform) {
          std::shared_ptr<WaveformOscillator> wOsc = std::dynamic_pointer_cast<WaveformOscillator>(osc);

          wOsc->frequency->value = values["freq"];

          // Handle both old format (amp/shift) and new format (minOutput/maxOutput)
          if (values.count("minOutput") > 0 && values.count("maxOutput") > 0) {
            // New format
            wOsc->minOutput->value = values["minOutput"];
            wOsc->maxOutput->value = values["maxOutput"];
          } else if (values.count("amp") > 0 && values.count("shift") > 0) {
            // Old format - convert to new format
            float amplitude = values["amp"];
            float shift = values["shift"];
            float halfRange = 0.3f * amplitude;  // OscillatorDampen * amplitude
            wOsc->minOutput->value = shift - halfRange;
            wOsc->maxOutput->value = shift + halfRange;
          }

          osc->enabled->boolValue = true;
        }
      } else {
        log("Couldn't find %s", paramId.c_str());
      }
    }
  }
}

void OscillationService::removeOscillatorsFor(std::vector<std::shared_ptr<Parameter>> parameters) {
  for (const auto& param : parameters) {
    auto it = oscillators.find(param->paramId);
    if (it != oscillators.end()) {
      it->second->enabled->boolValue = false; // Disable the oscillator
      oscillators.erase(it); // Remove the oscillator from the map
    }
  }
}

std::set<std::shared_ptr<Oscillator>> OscillationService::activeOscillators() {
  std::set<std::shared_ptr<Oscillator>> activeOscillators;
  
  for (const auto& [key, osc] : oscillators) {
    if (osc->enabled->boolValue) {
      activeOscillators.insert(osc);
    }
  }
  return activeOscillators;
}
