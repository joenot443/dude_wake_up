//
//  ConfigService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#ifndef ConfigService_hpp
#define ConfigService_hpp

#include <stdio.h>

#include "AvailableShaderChainer.hpp"
#include "AvailableShaderConfig.hpp"
#include "ShaderChainer.hpp"
#include "observable.hpp"
#include "ShaderType.hpp"


using json = nlohmann::json;

static const std::string MidiJsonKey = "midi";
static const std::string OscJsonKey = "osc";
static const std::string SourcesJsonKey = "sources";
static const std::string ShaderChainersJsonKey = "chainers";
static const std::string NameJsonKey = "name";

static const std::string ConfigTypeKey = "configType";
static const std::string ConfigTypeFull = "full";
static const std::string ConfigTypeAtomic = "atomic";

static const std::string ConfigFolderName = "shader_configs";


class ConfigService {
private:
  observable::subject<void()> configUpdateSubject;
public:
  void notifyConfigUpdate();
  void subscribeToConfigUpdates(std::function<void()> callback);
  
  // Chainers
  void saveShaderChainerConfigFile(std::shared_ptr<ShaderChainer> chainer,
                                   std::string path);
  void saveShaderConfigFile(Shader *shader,
                                   std::string name);
  bool validateShaderChainerJson(std::string path);
  void loadShaderChainerFile(std::string path);
  AvailableShaderChainer availableShaderChainerFromPath(std::string path);

  // Shaders
  std::vector<std::string> shaderConfigFoldersPaths();
  std::vector<AvailableShaderConfig> availableConfigsForShaderType(ShaderType type);
  std::string shaderConfigFolderForType(ShaderType type);
  json shaderConfigForPath(std::string path);
  void saveDefaultConfigFile();
  void loadDefaultConfigFile();

  void saveConfigFile(std::string path);
  void loadConfigFile(std::string path);

  
  std::string nottawaFolderFilePath();
  std::string relativeFilePathWithinNottawaFolder(std::string filePath);

  json jsonFromParameters(std::vector<Parameter *> parameters);

  static ConfigService *service;
  ConfigService(){};
  static ConfigService *getService() {
    if (!service) {
      service = new ConfigService;
    }
    return service;
  }
};

#endif /* ConfigService_hpp */
