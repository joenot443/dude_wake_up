//
//  FavoriteParameter.h
//  dude_wake_up
//
//  Created by Joe Crozier on 3/6/24.
//

#ifndef FavoriteParameter_h
#define FavoriteParameter_h

#include <string>
#include "json.hpp"
#include "Parameter.hpp"
#include "JSONSerializable.hpp"

struct FavoriteParameter: public JSONSerializable {
  std::string settingsName;
  std::string name;
  std::shared_ptr<Parameter> parameter;

  FavoriteParameter(std::shared_ptr<Parameter> parameter) : parameter(parameter), settingsName(parameter->ownerName), name(parameter->name) {};

  void load(json j);
  
  json serialize() {
    json j;
    j["settingsName"] = settingsName;
    j["name"] = name;
    j["paramId"] = parameter->paramId;
    return j;
  }
};

#endif /* FavoriteParameter_h */
