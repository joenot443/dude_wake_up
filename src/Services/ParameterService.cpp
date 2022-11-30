//
//  ParameterService.c
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#include "ParameterService.hpp"
#include "Console.hpp"

std::shared_ptr<Parameter> ParameterService::parameterForId(std::string paramId) {
  if (parameterMap.count(paramId) == 0) {
    return NULL;
  }
  return parameterMap[paramId];
}

void ParameterService::registerParameter(std::shared_ptr<Parameter> parameter) {
  if (parameterMap.count(parameter->paramId) != 0) {
    log("Reregistering Parameter %s", parameter->paramId.c_str());
  }
  parameterMap[parameter->paramId] = parameter;
}
