//
//  ParameterService.c
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#include "ConfigService.hpp"
#include "ParameterService.hpp"
#include "Console.hpp"

void ParameterService::setup() {
}


void ParameterService::notifyFavoritesUpdate()
{
  configUpdateSubject.notify();
}

void ParameterService::subscribeToFavoritesUpdates(std::function<void()> callback)
{
  configUpdateSubject.subscribe(callback);
}

std::vector<std::shared_ptr<Parameter>> ParameterService::allActiveParameters() {
  std::vector<std::shared_ptr<Parameter>> parameters;
  for (auto const &[key, val] : parameterMap) {
    std::shared_ptr<Parameter> param = val;
    if (param == nullptr) continue;
    
    if (param->active && !param->hiddenFromNode) {
      parameters.push_back(val);
    }
  }
  
  // Sort the parameters by settingsName, then by name
  std::sort(parameters.begin(), parameters.end(), [](std::shared_ptr<Parameter> a, std::shared_ptr<Parameter> b) {
    if (a->ownerSettingsId == b->ownerSettingsId) {
      return a->name < b->name;
    }
    if (a->ownerName == b->ownerName) {
      return a->paramId < b->paramId;
    }
    return a->ownerName < b->ownerName;
  });
  
  return parameters;
}

std::shared_ptr<Parameter>
ParameterService::parameterForId(std::string paramId) {
  if (parameterMap.count(paramId) == 0) {
    return nullptr;
  }
  
  if (auto shared_val = parameterMap[paramId]) {
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
  parameterMap[parameter->paramId] = parameter;
  if (parameter->favorited) {
    addFavoriteParameter(parameter);
  }
}

void ParameterService::tickParameters() {
  for (auto const &[key, shared_val] : parameterMap) {
    shared_val->active = false;
    shared_val->tick();
  }
}

std::vector<std::shared_ptr<FavoriteParameter>> ParameterService::favoriteParameters() {
  std::vector<std::shared_ptr<FavoriteParameter>> parameters;
  for (auto const &[key, val] : favoriteParameterMap) {
    parameters.push_back(val);
  }
  
  // Sort the parameters by settingsName, then by name
  std::sort(parameters.begin(), parameters.end(), [](std::shared_ptr<FavoriteParameter> a, std::shared_ptr<FavoriteParameter> b) {
    if (a->settingsName == b->settingsName) {
      return a->name < b->name;
    }
    return a->settingsName < b->settingsName;
  });
  
  return parameters;
}

void ParameterService::clear() {
  favoriteParameterMap.clear();
}

std::shared_ptr<FavoriteParameter> ParameterService::addFavoriteParameter(std::shared_ptr<Parameter> parameter) {
  if (parameter == nullptr) {
    log("Attempting to add null parameter");
    return nullptr;
  }
  parameter->favorited = true;
  favoriteParameterMap[parameter->paramId] = std::make_shared<FavoriteParameter>(parameter);
  return favoriteParameterMap[parameter->paramId];
}

bool ParameterService::hasFavoriteParameterFor(std::shared_ptr<Parameter> parameter) {
  if (parameter == nullptr) {
    log("Attempting to add null parameter");
    return false;
  }
  return favoriteParameterMap.count(parameter->paramId) != 0;
}

void ParameterService::removeFavoriteParameter(std::shared_ptr<Parameter> parameter) {
  if (parameter == nullptr) {
    log("Attempting to add null parameter");
    return;
  }
  parameter->favorited = false;
  favoriteParameterMap.erase(parameter->paramId);
}

/* 
  Expect the format of the json to be:
  {
    "favoriteParameters": [
      {
        "settingsName": "Favorite",
        "name": "Favorite",
        "paramId": "Favorite"
      }
    ]
  }
*/

void ParameterService::loadConfig(json j) {
  if (j.is_object()) {
    std::vector<json> favoriteParameters = j[FavoriteParameterJsonKey];
    for (auto const &obj : favoriteParameters) {
      if (parameterMap.count(obj["paramId"]) != 0) {
        auto favorite = addFavoriteParameter(parameterMap[obj["paramId"]]);
        favorite->load(obj);
      }
    }
    favoriteShaderTypes.clear();
    std::vector<int> intFavoriteTypes = j[FavoriteShadersJsonKey];
    for (auto const &type : intFavoriteTypes) {
      favoriteShaderTypes.insert(static_cast<ShaderType>(type));
    }
    
    std::vector<std::string> stageIds = j[StageShadersJsonKey];
    for (auto const &id : stageIds) {
      stageShaderIds.insert(id);
    }
  }
}

json ParameterService::config() {
  json j;
  std::vector<json> favoriteParameters = {};
  for (auto const &[key, param] : favoriteParameterMap) {
    if (param != nullptr) {
      favoriteParameters.push_back(param->serialize());
    }
  }
  j[FavoriteParameterJsonKey] = favoriteParameters;
  j[FavoriteShadersJsonKey] = favoriteShaderTypes;
  j[StageShadersJsonKey] = stageShaderIds;
  return j;
}

void ParameterService::addFavoriteShaderType(ShaderType type) {
  if (favoriteShaderTypes.count(type) == 0) {
    favoriteShaderTypes.insert(type);
  }
}

void ParameterService::removeFavoriteShaderType(ShaderType type) {
  if (favoriteShaderTypes.count(type) != 0) {
    favoriteShaderTypes.erase(type);
  }
}

void ParameterService::toggleFavoriteShaderType(ShaderType type) {
  if (favoriteShaderTypes.count(type) != 0) {
    favoriteShaderTypes.erase(type);
  } else {
    favoriteShaderTypes.insert(type);
  }
  ConfigService::getService()->saveDefaultConfigFile();
  notifyFavoritesUpdate();
}

bool ParameterService::isShaderTypeFavorited(ShaderType type) {
  return favoriteShaderTypes.count(type) != 0;
}

void ParameterService::addStageShaderId(std::string shaderId) {
    stageShaderIds.insert(shaderId);
}

bool ParameterService::isShaderIdStage(std::string shaderId) {
    return stageShaderIds.count(shaderId) != 0;
}

void ParameterService::removeStageShaderId(std::string shaderId) {
    stageShaderIds.erase(shaderId);
}

void ParameterService::toggleStageShaderId(std::string shaderId) {
    if (stageShaderIds.count(shaderId) != 0) {
        stageShaderIds.erase(shaderId);
    } else {
        stageShaderIds.insert(shaderId);
    }
    ConfigService::getService()->saveDefaultConfigFile();
    notifyFavoritesUpdate();
}
