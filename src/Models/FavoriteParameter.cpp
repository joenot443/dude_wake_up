//
//  FavoriteParameter.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/6/24.
//

#include <stdio.h>
#include "json.hpp"
#include "FavoriteParameter.hpp"
#include "ParameterService.hpp"

void FavoriteParameter::load(json j) {
  parameter = ParameterService::getService()->parameterForId(j["paramId"]);
  settingsName = j["settingsName"];
  name = j["name"];
}
