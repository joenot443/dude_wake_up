//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"

std::vector<std::shared_ptr<ShaderChainer>> ShaderChainerService::shaderChainers() {
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers;
  for (auto const& [key, val] : shaderChainerMap) {
    shaderChainers.push_back(val);
  }
  // Sort the ShaderChainers by their names
  std::sort(shaderChainers.begin(), shaderChainers.end(), [](const std::shared_ptr<ShaderChainer> &a, const std::shared_ptr<ShaderChainer> &b) {
    return a->name < b->name;
  });
  
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

int ShaderChainerService::count() {
  return shaderChainerMap.size();
}

void ShaderChainerService::selectShaderChainer(::shared_ptr<ShaderChainer> shaderChainer) {
  selectedShaderChainer = shaderChainer;
  // When we select a ShaderChainer, we deselect the Shader
  selectedShader = nullptr;
}

void ShaderChainerService::selectShader(std::shared_ptr<Shader> shader) {
  selectedShader = shader;
}

void ShaderChainerService::addShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer) {
  if (shaderChainerMap.count(shaderChainer->chainerId) != 0) {
    log("Reregistering ShaderChainer %s", shaderChainer->chainerId.c_str());
  }
  
  // Select the first ShaderChainer to be created
  if (shaderChainerMap.empty()) {
    selectShaderChainer(shaderChainer);
  }
  
  shaderChainerMap[shaderChainer->chainerId] = shaderChainer;
  
  VideoSourceService::getService()->addVideoSource(shaderChainer);
}

// ConfigurableService

json ShaderChainerService::config() {
  json container;
  
  for (auto & pair : shaderChainerMap) {
    container[pair.first] = pair.second->serialize();
  }
  
  return container;
}

void ShaderChainerService::loadConfig(json data) {
  shaderChainerMap.clear();
  
  if (data.is_object()) {
    std::map<std::string, json> items = data;

    for (auto const & pair : items) {
      std::string chainerName = pair.second["name"];
      
      auto videoSources = VideoSourceService::getService()->videoSources();
      
      
      auto shaderChainer = new ShaderChainer(pair.first, chainerName, videoSources.at(0));
      shaderChainer->load(pair.second);
      shaderChainer->setup();
      addShaderChainer(std::shared_ptr<ShaderChainer>(shaderChainer));
    }
  }
}
