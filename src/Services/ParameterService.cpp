//
//  ParameterService.c
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#include "ParameterService.hpp"
#include "Console.hpp"

std::shared_ptr<Parameter>
ParameterService::parameterForId(std::string paramId) {
  if (parameterMap.count(paramId) == 0) {
    return nullptr;
  }
  
  if (auto shared_val = parameterMap[paramId].lock()) {
    return shared_val;
  } else {
    log("Requesting a deleted Parameter");
    return nullptr;
  }
}

void ParameterService::registerParameter(std::shared_ptr<Parameter> parameter) {
  if (parameter == nullptr) {
    log("Attempting to add null parameter");
    return;
  }
  if (parameterMap.count(parameter->paramId) != 0) {
//    log("Reregistering Parameter %s", parameter->paramId.c_str());
  }
//  log("Registering Parameter %s", parameter->paramId.c_str());
  parameterMap[parameter->paramId] = std::weak_ptr<Parameter>(parameter);

}

void ParameterService::tickParameters() {
  for (auto const &[key, weak_val] : parameterMap) {
      if (auto shared_val = weak_val.lock()) {
        shared_val->tick();
      }
  }
}
