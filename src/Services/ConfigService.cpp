//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "ParameterService.h"
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
    std::string settingsId = data["settingsId"];
    auto shaderChainer = new ShaderChainer(settingsId);
    shaderChainer->load(data);
    ParameterService::getService()->loadParameters(&data);
    log("Loaded config file.");
    return shaderChainer;
  }
  return nullptr;
}

ShaderSettings* ConfigService::settingsFromJson(json j) {
  ShaderType shaderType = j["shaderType"];
  std::string settingsId = j["settingsId"];
  switch (shaderType) {
    case ShaderTypeNone:
      return new ShaderSettings(settingsId);
    case ShaderTypeHSB: {
      auto settings = new HSBSettings(settingsId);
      settings->load(j);
      return settings;
    }
    case ShaderTypePixelate: {
      auto settings = new PixelSettings(settingsId);
      settings->load(j);
      return settings;
    }
    case ShaderTypeGlitch: {
      auto settings = new HSBSettings(settingsId);
      settings->load(j);
      return settings;
    }
    case ShaderTypeFeedback: {
      auto settings = new FeedbackSettings(settingsId, 0);
      settings->load(j);
      return settings;
    }
    case ShaderTypeBlur: {
      auto settings = new BlurSettings(settingsId);
      settings->load(j);
      return settings;
    }
  }
}
