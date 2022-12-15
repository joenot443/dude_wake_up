//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "ParameterService.hpp"
#include "OscillationService.hpp"
#include "VideoSourceService.hpp"
#include "ShaderChainerService.hpp"
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

static const std::string MidiJsonKey = "midi";
static const std::string OscJsonKey = "osc";
static const std::string SourcesJsonKey = "sources";
static const std::string ShaderChainersJsonKey = "chainers";


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


void ConfigService::loadDefaultConfigFile() {
  const char* homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  loadConfigFile(path);
}

void ConfigService::saveDefaultConfigFile() {
  const char* homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  saveConfigFile(path);
}

void ConfigService::saveConfigFile(std::string path) {
  json config;
  
  config[MidiJsonKey] = MidiService::getService()->config();
  config[OscJsonKey] = OscillationService::getService()->config();
  config[ShaderChainersJsonKey] = ShaderChainerService::getService()->config();
  config[SourcesJsonKey] = VideoSourceService::getService()->config();
  
  std::ofstream fileStream;
  fileStream.open(path.c_str(), std::ios::trunc);
  
  if (fileStream.is_open()) {
    std::cout << config.dump(4) << std::endl;
    fileStream << config.dump(4);
    fileStream.close();
    
    std::cout << "Successfully saved config" << std::endl;
  } else {
    std::cout << config.dump(4) << std::endl;
    log("Problem saving config.");
  }
  
}

void ConfigService::loadConfigFile(std::string path) {
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  json data;
  
  if (fileStream.is_open()) {
    
    try {
      fileStream >> data;
    }
    catch (int code) {
      log("Failed to load JSON file.");
      return;
    }
  }
  
  std::cout << data.dump(4) << std::endl;
  
  if (!data.is_object()) {
    log("Failed to parse JSON file.");
  }
  
  if (data[MidiJsonKey].is_object()) {
    MidiService::getService()->loadConfig(data[MidiJsonKey]);
  }

  if (data[OscJsonKey].is_object()) {
    OscillationService::getService()->loadConfig(data[OscJsonKey]);
  }
  
  if (data[SourcesJsonKey].is_object()) {
    VideoSourceService::getService()->loadConfig(data[SourcesJsonKey]);
  }
  
  if (data[ShaderChainersJsonKey].is_object()) {
    ShaderChainerService::getService()->loadConfig(data[ShaderChainersJsonKey]);
  }
}
