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
#include "Models/Strand.hpp"
#include "AvailableShaderConfig.hpp"
#include "observable.hpp"
#include "ShaderType.hpp"
#include "Workspace.hpp"

class Shader;
class Parameter;

using json = nlohmann::json;
static const std::string ConfigFolderName = "shader_configs";

static const std::string MidiJsonKey = "midi";
static const std::string OscJsonKey = "osc";
static const std::string SourcesJsonKey = "sources";
static const std::string ShadersJsonKey = "shaders";
static const std::string ConnectionsJsonKey = "connections";
static const std::string NameJsonKey = "name";
static const std::string PreviewPathJsonKey = "preview";
static const std::string LayoutJsonKey = "layout";
static const std::string ParametersJsonKey = "parameters";
static const std::string WorkspaceJsonKey = "workspace";
static const std::string StrandsJsonKey = "strands";
static const std::string AudioJsonKey = "audio";

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
  std::string fileBrowserPath;
  
public:
  void notifyConfigUpdate();
  void subscribeToConfigUpdates(std::function<void()> callback);
  void setup();
  
  // Strands
  void saveStrandFile(Strand strand, std::string path, std::string previewPath);
  bool validateStrandJson(std::string path);
  std::vector<std::string> loadStrandFile(std::string path);
  
  AvailableStrand availableStrandFromPath(std::string path);
  
  // Icons
  std::vector<std::string> availableIconFilenames();
  
  // Shader Configs
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
  
  // Static File Paths
  std::string nottawaFolderFilePath();
  std::string libraryFolderFilePath();
  std::string exportsFolderFilePath();
  std::string templatesFolderFilePath();
  std::string strandsFolderFilePath();
  std::string defaultConfigFilePath();
  std::string relativeFilePathWithinNottawaFolder(std::string filePath);
  
  json jsonFromParameters(std::vector<Parameter *> parameters);
  
  // Workspaces ("full" config files")
  std::shared_ptr<Workspace> currentWorkspace;
  bool isEditingWorkspace();
  void saveCurrentWorkspace();
  void saveNewWorkspace();
  void updateWorkspace(std::shared_ptr<Workspace> workspace);
  void saveWorkspace(std::shared_ptr<Workspace> workspace);
  bool loadWorkspaceDialogue();
  void loadWorkspace(std::shared_ptr<Workspace> workspace);
  void closeWorkspace();
  
  static std::string appSupportFilePath();
  
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
