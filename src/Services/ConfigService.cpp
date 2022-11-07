//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "ParameterService.h"
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
    auto videoSources = VideoSourceService::getService()->videoSources();
    auto shaderChainer = new ShaderChainer(chainerId, videoSources.at(0));
    shaderChainer->load(data);
    log("Loaded config file.");
    return shaderChainer;
  }
  return nullptr;
}

ShaderChainer ConfigService::shaderChainerFromJson(json j) {
  std::string chainerId = j["chainerId"];
  auto videoSources = VideoSourceService::getService()->videoSources();
  
  auto shaderChainer = ShaderChainer(chainerId, videoSources.at(0));
  shaderChainer.load(j);
  return shaderChainer;
}

//Shader ConfigService::shaderFromJson(json j) {
//  ShaderType shaderType = j["shaderType"];
//  std::string shaderId = std::to_string((int) j["shaderId"]);
//  switch (shaderType) {
//    case ShaderTypeNone: {
//      auto settings = new ShaderSettings(shaderId);
//      return Shader(settings);
//    }
//    case ShaderTypeHSB: {
//      auto settings = new HSBSettings(shaderId, j);
//      settings->load(j);
//      return HSBShader(settings);
//    }
//    case ShaderTypePixelate: {
//      auto settings = new PixelSettings(shaderId, j);
//      settings->load(j);
//      return PixelShader(settings);
//    }
//    case ShaderTypeGlitch: {
//      auto settings = new HSBSettings(shaderId, j);
//      settings->load(j);
//      return HSBShader(settings);
//    }
//    case ShaderTypeFeedback: {
//      auto settings = new FeedbackSettings(shaderId, j);
//      settings->load(j);
//      return FeedbackShader(settings);
//    }
//    case ShaderTypeBlur: {
//      auto settings = new BlurSettings(shaderId, j);
//      settings->load(j);
//      return BlurShader(settings);
//    }
//    case ShaderTypeMirror: {
//      auto settings = new MirrorSettings(shaderId, j);
//      settings->load(j);
//      return MirrorShader(settings);
//    }
//    case ShaderTypeTransform: {
//      auto settings = new TransformSettings(shaderId, j);
//      settings->load(j);
//      return TransformShader(settings);
//    }
//  }
//}
