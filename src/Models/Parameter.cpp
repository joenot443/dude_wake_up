//
//  Parameter.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-25.
//

#include <stdio.h>
#include "Strings.hpp"
#include "Parameter.hpp"
#include "ParameterService.hpp"
#include <memory>
#include "UUID.hpp"

Parameter::Parameter(std::string name, float value, ParameterType type) :
name(name),
defaultValue(value),
type(type),
value(value),
min(0.0),
max(1.0),
intValue(static_cast<int>(value)),
boolValue(value > 0.0001),
driver(NULL),
shift(NULL),
scale(NULL) {
  paramId = UUID::generateParamId(name);
};

Parameter::Parameter(std::string name,
                     float value,
                     float min,
                     float max,
                     ParameterType type) :
name(name),
type(type),
defaultValue(value),
value(value),
min(min),
max(max),
boolValue(value > 0.0001),
intValue(static_cast<int>(value)),
driver(NULL),
shift(NULL),
scale(NULL) {
  paramId = UUID::generateParamId(name);
  intValue = static_cast<int>(value);
  boolValue = static_cast<bool>(value);
};

void Parameter::tick() {
  if (driver != nullptr && paramId.length() > 5) {
    setValue(driver->value * scale->value + shift->value);
  }
}

