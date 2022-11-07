//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"

std::vector<std::shared_ptr<ShaderChainer>> ShaderChainerService::shaderChainers() {
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers;
  for (auto const& [key, val] : shaderChainerMap) {
    shaderChainers.push_back(val);
  }
  return shaderChainers;
}

std::vector<std::string> ShaderChainerService::shaderChainerNames() {
  std::vector<std::string> names;
  for (auto const& [key, val] : shaderChainerMap) {
    names.push_back(val->name);
  }
  return names;
}

void ShaderChainerService::updateShaderChainers() {
  for (auto const& [key, val] : shaderChainerMap) {
    val->update();
  }
}

void ShaderChainerService::removeShaderChainer(std::string id) {
  if (shaderChainerMap.count(id) == 0) {
    log("Tried to remove ShaderChainer %s, but it doesn't exist", id.c_str());
    return;
  }
  shaderChainerMap.erase(id);
}

void ShaderChainerService::addShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer) {
  if (shaderChainerMap.count(shaderChainer->chainerId) != 0) {
    log("Reregistering ShaderChainer %s", shaderChainer->chainerId.c_str());
  }
  shaderChainerMap[shaderChainer->chainerId] = shaderChainer;
}
