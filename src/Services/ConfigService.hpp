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
#include "ShaderChainer.hpp"
#include "observable.hpp"


using json = nlohmann::json;

static const std::string MidiJsonKey = "midi";
static const std::string OscJsonKey = "osc";
static const std::string SourcesJsonKey = "sources";
static const std::string ShaderChainersJsonKey = "chainers";

static const std::string ConfigTypeKey = "configType";
static const std::string ConfigTypeFull = "full";
static const std::string ConfigTypeAtomic = "atomic";


class ConfigService {
private:
  observable::subject<void()> configUpdateSubject;
public:
  void notifyConfigUpdate();
  void subscribeToConfigUpdates(std::function<void()> callback);
  
  // Shaders
  void saveShaderChainerConfigFile(std::shared_ptr<ShaderChainer> chainer,
                                   std::string path);
  bool validateShaderChainerJson(std::string path);
  AvailableShaderChainer availableShaderChainerFromPath(std::string path);

  json jsonFromParameters(std::vector<Parameter *> parameters);

  void saveDefaultConfigFile();
  void loadDefaultConfigFile();

  void saveConfigFile(std::string path);
  void loadConfigFile(std::string path);

  ofDirectory nottawaFolderFilePath();
  std::string relativeFilePathWithinNottawaFolder(std::string filePath);
  
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
