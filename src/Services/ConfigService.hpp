//
//  ConfigService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#ifndef ConfigService_hpp
#define ConfigService_hpp

#include <stdio.h>

#include "AvailableStrand.hpp"
#include "Strand.hpp"
#include "AvailableShaderConfig.hpp"
#include "ShaderChainer.hpp"
#include "observable.hpp"
#include "ShaderType.hpp"


using json = nlohmann::json;
static const std::string ConfigFolderName = "shader_configs";

static const std::string MidiJsonKey = "midi";
static const std::string OscJsonKey = "osc";
static const std::string SourcesJsonKey = "sources";
static const std::string ShadersJsonKey = "shaders";
static const std::string ConnectionsJsonKey = "connections";
static const std::string NameJsonKey = "name";
static const std::string LayoutJsonKey = "layout";

static const std::string ConfigTypeKey = "configType";
static const std::string ConfigTypeFull = "full";
static const std::string ConfigTypeStrand = "strand";

/*
 Full Config:
 {
  "type": full,
  "midi" : {}
  "osc" : {}
  "sources" : {}
  "shaders" : {}
  "connections" : {}
  "layout": {}
 }
 */

/*
 Strand:
 {
  "type": strand,
  "name": "foo",
  "sources": {},
  "shaders": {},
  "connections: {},
 }
 */

class ConfigService {
private:
  observable::subject<void()> configUpdateSubject;
public:
  void notifyConfigUpdate();
  void subscribeToConfigUpdates(std::function<void()> callback);
  
  // Strands
  void saveStrandFile(Strand strand, std::string path);
  bool validateStrandJson(std::string path);
  void loadStrandFile(std::string path);
  
  AvailableStrand availableStrandFromPath(std::string path);
  
  // Icons
  std::vector<std::string> availableIconFilenames();
  
  // Shaders
  std::vector<std::string> shaderConfigFoldersPaths();
  std::vector<AvailableShaderConfig> availableConfigsForShaderType(ShaderType type);
  std::string shaderConfigFolderForType(ShaderType type);
  json shaderConfigForPath(std::string path);
  void saveShaderConfigFile(std::shared_ptr<Shader> shader, std::string name);
  void checkAndSaveDefaultConfigFile();
  void saveDefaultConfigFile();
  void loadDefaultConfigFile();

  void saveConfigFile(std::string path);
  void loadConfigFile(std::string path);
  json currentConfig();
  
  std::string nottawaFolderFilePath();
  std::string libraryFolderFilePath();
  std::string relativeFilePathWithinNottawaFolder(std::string filePath);

  json jsonFromParameters(std::vector<Parameter *> parameters);
  
  bool isLoading;
  
  static ConfigService *service;
  ConfigService(){};
  
  json lastConfig = 0;
  
  static ConfigService *getService() {
    if (!service) {
      service = new ConfigService;
    }
    return service;
  }
};

#endif /* ConfigService_hpp */
