//
//  Parameter.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-25.
//

#include <stdio.h>
#include "Strings.hpp"
#include "Parameter.hpp"
#include "ParameterService.h"


Parameter::Parameter(std::string name, std::string settingsId, float value) : name(name), defaultValue(value), value(value)  {
  paramId = formatString("%s-%s", settingsId.c_str(), name.c_str());
  ParameterService::getService()->registerParameter(this);
};

Parameter::Parameter(std::string name, std::string settingsId, std::string shaderKey, float value): name(name), shaderKey(shaderKey), defaultValue(value), value(value) {
  paramId = formatString("%s-%s", settingsId.c_str(), name.c_str());
  ParameterService::getService()->registerParameter(this);
};

Parameter::Parameter(std::string name, std::string settingsId, std::string shaderKey, float value, float min, float max) : name(name), shaderKey(shaderKey), defaultValue(value), value(value), min(min), max(max) {
  paramId = formatString("%s-%s", settingsId.c_str(), name.c_str());
  ParameterService::getService()->registerParameter(this);
};

Parameter::Parameter(std::string name, std::string settingsId, float value, float min, float max) : name(name), defaultValue(value), value(value), min(min), max(max) {
  paramId = formatString("%s-%s", settingsId.c_str(), name.c_str());
  ParameterService::getService()->registerParameter(this);
};

