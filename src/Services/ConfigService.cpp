//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "BlurShader.hpp"
#include "Console.hpp"
#include "EmptyShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "MirrorShader.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "PixelShader.hpp"
#include "ShaderChainerService.hpp"
#include "TransformShader.hpp"
#include "VideoSourceService.hpp"
#include "json.hpp"
#include <fstream>
#include <ostream>

void ConfigService::notifyConfigUpdate() {
  configUpdateSubject.notify();
}

void ConfigService::subscribeToConfigUpdates(std::function<void()> callback) {
  configUpdateSubject.subscribe(callback);
}

ofDirectory ConfigService::nottawaFolderFilePath() {
  auto path = ofFilePath::join(ofFilePath::getUserHomeDir(), "/nottawa");
  return ofDirectory(path);
}

std::string ConfigService::relativeFilePathWithinNottawaFolder(std::string filePath) {
  return ofFilePath::join(nottawaFolderFilePath().getAbsolutePath(), filePath);
}

json jsonFromParameters(std::vector<Parameter *> parameters) {
  json j;
  for (auto p : parameters) {
    j[p->paramId.c_str()] = p->value;
  }
}

void ConfigService::saveShaderChainerConfigFile(
    std::shared_ptr<ShaderChainer> chainer, std::string name) {
  std::ofstream fileStream;
  auto filePath = relativeFilePathWithinNottawaFolder(name);
  
  fileStream.open(filePath.c_str(), std::ios::trunc);
  json container;

  container[ShaderChainersJsonKey] = chainer->serialize();
  container[SourcesJsonKey] = chainer->source->serialize();
  container[ConfigTypeKey] = ConfigTypeAtomic;

  if (fileStream.is_open()) {
    std::cout << container.dump(4) << std::endl;
    fileStream << container.dump(4);
    fileStream.close();
  } else {
    log("Problem saving config.");
  }
  
  notifyConfigUpdate();
}

bool ConfigService::validateShaderChainerJson(std::string path) {
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  if (fileStream.is_open()) {
    json data;
    try {
      fileStream >> data;
    } catch (int code) {
      log("Failed to load JSON file.");
      return;
    }
    if (data[ConfigTypeKey] != ConfigTypeAtomic || !data.contains("chainers") || !data.contains("sources")) { return false; }

    return true;
  }
}

AvailableShaderChainer
ConfigService::availableShaderChainerFromPath(std::string path) {
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  if (fileStream.is_open()) {
    json data;
    try {
      fileStream >> data;
    } catch (int code) {
      log("Failed to load JSON file.");
      return;
    }
    std::string chainerName = data["chainers"]["name"];

    return AvailableShaderChainer(chainerName, path);
  }
}

void ConfigService::loadDefaultConfigFile() {
  const char *homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  loadConfigFile(path);
}

void ConfigService::saveDefaultConfigFile() {
  const char *homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  saveConfigFile(path);
}

void ConfigService::saveConfigFile(std::string path) {
  json config;

  config[MidiJsonKey] = MidiService::getService()->config();
  config[OscJsonKey] = OscillationService::getService()->config();
  config[ShaderChainersJsonKey] = ShaderChainerService::getService()->config();
  config[SourcesJsonKey] = VideoSourceService::getService()->config();
  config[ConfigTypeKey] = ConfigTypeFull;
  
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
    } catch (int code) {
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
