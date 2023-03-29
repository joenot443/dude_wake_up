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
#include "UUID.hpp"

Parameter::Parameter(std::string name, float value) :
name(name),
defaultValue(value),
value(value),
min(0.0),
max(1.0),
intValue(static_cast<int>(value)),
boolValue(value > 0.0001)
{
  paramId = UUID::generateParamId(name);
  std::shared_ptr<Parameter> sharedSelf = std::shared_ptr<Parameter>(this);
  ParameterService::getService()->registerParameter(sharedSelf);
};

Parameter::Parameter(std::string name,
                     float value,
                     float min,
                     float max) :
name(name),
defaultValue(value),
value(value),
min(min),
max(max),
boolValue(value > 0.0001)
{
  paramId = UUID::generateParamId(name);
  std::shared_ptr<Parameter> sharedSelf = std::shared_ptr<Parameter>(this);
  ParameterService::getService()->registerParameter(sharedSelf);
};
