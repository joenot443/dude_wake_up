//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "ParameterService.hpp"
#include "VideoSourceService.hpp"
#include "HSBShader.hpp"
#include "PixelShader.hpp"
#include "GlitchShader.hpp"
#include "BlurShader.hpp"
#include "TransformShader.hpp"
#include "MirrorShader.hpp"
#include "EmptyShader.hpp"
#include "json.hpp"
#include "Console.hpp"
#include <ostream>
#include <fstream>

json jsonFromParameters(std::vector<Parameter*> parameters) {
  json j;
  for (auto p : parameters) {
    j[p->paramId.c_str()] = p->value;
  }
}

void ConfigService::saveShaderChainerConfigFile(ShaderChainer* chainer, std::string path) {
  std::ofstream fileStream;
  fileStream.open(path.c_str(), std::ios::trunc);
  json container;
  container = chainer->serialize();
  
  if (fileStream.is_open()) {
    std::cout << container.dump(4) << std::endl;
    fileStream << container.dump(4);
    fileStream.close();
  } else {
    log("Problem saving config.");
  }
}

ShaderChainer* ConfigService::loadShaderChainerConfigFile(std::string name) {
  std::fstream fileStream;
  fileStream.open(name, std::ios::in);
  if (fileStream.is_open()) {
    json data;
    try {
      fileStream >> data;
    }
    catch (int code) {
      log("Failed to load JSON file.");
      return;
    }
    std::string chainerId = data["chainerId"];
    std::string chainerName = data["name"];
    auto videoSources = VideoSourceService::getService()->videoSources();
    auto shaderChainer = new ShaderChainer(chainerId, name, videoSources.at(0));
    shaderChainer->load(data);
    log("Loaded config file.");
    return shaderChainer;
  }
  return nullptr;
}
