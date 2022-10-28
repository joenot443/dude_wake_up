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
#include <string>

class ShaderSettings: public JSONSerializable {

public:
  std::string settingsId;
  ShaderSettings(std::string settingsId) : settingsId(settingsId) {}
  
  void load(json j) {
    for (auto p : parameters) {
      p->setValue(j[p->paramId]);
    }
    for (auto o : oscillators) {
      o->frequency.setValue(j[o->frequency.paramId.c_str()]);
      o->amplitude.setValue(j[o->amplitude.paramId.c_str()]);
      o->shift.setValue(j[o->shift.paramId.c_str()]);
    }
  }
};

#endif
