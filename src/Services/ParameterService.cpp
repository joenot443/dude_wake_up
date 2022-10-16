//
//  ParameterService.c
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#include "ParameterService.h"
#include "Console.hpp"

Parameter * ParameterService::parameterForId(std::string paramId) {
  if (parameterMap.count(paramId) == 0) {
    return NULL;
  }
  return parameterMap[paramId];
}

void ParameterService::registerParameter(Parameter *parameter) {
  if (parameterMap.count(parameter->paramId) != 0) {
    log("Reregistering Parameter %s", parameter->paramId.c_str());
  }
  parameterMap[parameter->paramId] = parameter;
}

void ParameterService::loadParameters(json * j) {
  for (json::iterator it = j->begin(); it != j->end(); ++it) {
    if (parameterForId(it.key())) {
      parameterMap[it.key()]->setValue(it.value());
    }
    std::cout << it.key() << " : " << it.value() << "\n";
  }
}
