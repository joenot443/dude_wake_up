//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "BlurShader.hpp"
#include "LayoutStateService.hpp"
#include "Console.hpp"
#include "EmptyShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "MirrorShader.hpp"
#include "OscillationService.hpp"
#include "BookmarkService.hpp"
#include "ParameterService.hpp"
#include "PixelShader.hpp"
#include "ShaderChainerService.hpp"
#include "StrandService.hpp"
#include "TransformShader.hpp"
#include "VideoSourceService.hpp"
#include "json.hpp"
#include "ofMain.h"
#include <fstream>
#include <ostream>


void ConfigService::notifyConfigUpdate()
{
  configUpdateSubject.notify();
}

void ConfigService::subscribeToConfigUpdates(std::function<void()> callback)
{
  configUpdateSubject.subscribe(callback);
}

std::string ConfigService::nottawaFolderFilePath()
{
  return appSupportFilePath() + "/nottawa";
}

std::string ConfigService::exportsFolderFilePath()
{
  return relativeFilePathWithinNottawaFolder("/exports");
}

std::string ConfigService::libraryFolderFilePath()
{
  return relativeFilePathWithinNottawaFolder("/videos/");
}

std::string ConfigService::strandsFolderFilePath()
{
  return relativeFilePathWithinNottawaFolder("/strands/");
}

std::string ConfigService::relativeFilePathWithinNottawaFolder(std::string filePath)
{
  return nottawaFolderFilePath() + filePath;
}

std::string ConfigService::appSupportFilePath() {
  std::string appSupportPath = ofFilePath::getUserHomeDir() + "/Library/Containers/com.joecrozier.nottawa/Data";
  
  // Create the directory if it doesn't exist
  if (!ofDirectory::doesDirectoryExist(appSupportPath, false)) {
    ofDirectory::createDirectory(appSupportPath, false, true);
  }
  
  return appSupportPath;
}

std::string ConfigService::templatesFolderFilePath()
{
  return "misc/templates";
}

json jsonFromParameters(std::vector<Parameter *> parameters)
{
  json j;
  for (auto p : parameters)
  {
    j[p->paramId.c_str()] = p->value;
  }
  return j;
}

std::string ConfigService::shaderConfigFolderForType(ShaderType type)
{
  return nottawaFolderFilePath() + formatString("/shaders/%s", shaderTypeName(type).c_str());
}

std::vector<std::string> ConfigService::shaderConfigFoldersPaths()
{
  std::vector<ShaderType> types = AllShaderTypes();
  std::vector<std::string> paths = {};
  for (auto type : types)
  {
    paths.push_back(shaderConfigFolderForType(type));
  }
  return paths;
}

std::vector<std::string> ConfigService::availableIconFilenames()
{
  std::vector<std::string> paths = {};
  ofDirectory directory;
  // Open the subdirectory for that ShaderType
  directory.open("shaders/images");
  directory.listDir();
  directory.sort();
  
  for (int i = 0; i < directory.size(); i++)
  {
    auto file = directory.getFile(i);
    bool isDirectory = file.isDirectory();
    if (isDirectory)
      continue;
    
    paths.push_back(file.getBaseName());
  }
  
  return paths;
}

json ConfigService::shaderConfigForPath(std::string path)
{
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  json data;
  
  if (fileStream.is_open())
  {
    try
    {
      fileStream >> data;
    }
    catch (int code)
    {
      log("Could not load JSON file for %s.", path.c_str());
      return 0;
    }
  }
  
  return data;
}

std::vector<AvailableShaderConfig> ConfigService::availableConfigsForShaderType(ShaderType type)
{
  ofDirectory directory;
  // Open the subdirectory for that ShaderType
  auto path = shaderConfigFolderForType(type);
  directory.open(path);
  directory.listDir();
  directory.sort();
  std::vector<AvailableShaderConfig> configs = {};
  
  for (int i = 0; i < directory.size(); i++)
  {
    auto file = directory.getFile(i);
    bool isDirectory = file.isDirectory();
    if (isDirectory)
      continue;
    
    bool isJson = ofIsStringInString(file.getFileName(), ".json");
    if (!isJson)
      continue;
    
    configs.push_back(AvailableShaderConfig(file.getFileName(), file.getAbsolutePath()));
  }
  return configs;
}

void ConfigService::saveStrandFile(Strand strand, std::string path, std::string previewPath)
{
  std::ofstream fileStream;
  
  fileStream.open(path.c_str(), std::ios::trunc);
  json container;
  
  container[ShadersJsonKey] = ShaderChainerService::getService()->config();
  container[SourcesJsonKey] = VideoSourceService::getService()->config();
  container[ConnectionsJsonKey] = ShaderChainerService::getService()->connectionsConfig();
  container[ConfigTypeKey] = ConfigTypeStrand;
  container[ParametersJsonKey] = ParameterService::getService()->config();
  container[PreviewPathJsonKey] = previewPath;
  
  if (fileStream.is_open())
  {
    std::cout << container.dump(4) << std::endl;
    fileStream << container.dump(4);
    fileStream.close();
  }
  else
  {
    log("Problem saving config.");
  }
  
  StrandService::getService()->addStrand(std::make_shared<AvailableStrand>(strand.name, path, previewPath));
  notifyConfigUpdate();
}

void ConfigService::saveShaderConfigFile(std::shared_ptr<Shader> shader,
                                         std::string name)
{
  std::ofstream fileStream;
  auto fileName = formatString("%s.json", name.c_str());
  auto filePath = ofFilePath::join(shaderConfigFolderForType(shader->type()), fileName);
  fileStream.open(filePath.c_str(), std::ios::trunc);
  json container = shader->serialize();
  
  if (fileStream.is_open())
  {
    std::cout << container.dump(4) << std::endl;
    fileStream << container.dump(4);
    fileStream.close();
  }
  else
  {
    log("Problem saving config.");
  }
  
  notifyConfigUpdate();
}

bool ConfigService::validateStrandJson(std::string path)
{
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  if (fileStream.is_open())
  {
    json data;
    try
    {
      fileStream >> data;
    }
    catch (...)
    {
      log("Chainer JSON deemed invalid");
      return false;
    }
    if (data[ConfigTypeKey] != ConfigTypeStrand)
    {
      return false;
    }
    
    return true;
  }
  return false;
}

AvailableStrand ConfigService::availableStrandFromPath(std::string path)
{
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  if (fileStream.is_open())
  {
    json data;
    try
    {
      fileStream >> data;
    }
    catch (int code)
    {
      log("Couldn't load available Strand for %s", path.c_str());
      return AvailableStrand("", "", "");
    }
    std::string name = data[NameJsonKey];
    std::string previewPath = data[PreviewPathJsonKey];
    
    return AvailableStrand(name, path, previewPath);
  }
  return AvailableStrand("", "", "");
}

// Checks if the lastConfig generated by has changed
void ConfigService::checkAndSaveDefaultConfigFile()
{
  // Only perform this check every 3000 frames, and after we've been launched for 10s
  if (ofGetFrameNum() % 3000 != 0 || ofGetFrameNum() < 1000)
    return;
  
  auto currentConfig = this->currentConfig();
  if (currentConfig != lastConfig)
  {
    saveDefaultConfigFile();
    lastConfig = currentConfig;
  }
}

std::string ConfigService::defaultConfigFilePath() {
  return formatString("%s/config.json", appSupportFilePath().c_str());
}

void ConfigService::loadDefaultConfigFile()
{
  BookmarkService::getService()->beginAccessingBookmark(defaultConfigFilePath());
  loadConfigFile(defaultConfigFilePath());
}

void ConfigService::saveDefaultConfigFile()
{
  saveConfigFile(defaultConfigFilePath());
}

void ConfigService::saveCurrentWorkspace() {
  // If we don't have a workspace, create a new one
  if (currentWorkspace == nullptr) {
    saveNewWorkspace();
    return;
  }
  
  saveConfigFile(currentWorkspace->path);
}

void ConfigService::saveNewWorkspace() {
  // Present a file dialog to save the config file
  // Use a default name of "CURRENT_DATE_TIME.json"
  std::string defaultName = ofGetTimestampString("Workspace-%m-%d-%Y.json");
  ofFileDialogResult result = ofSystemSaveDialog(defaultName, "Save Workspace");
  if (result.bSuccess)
  {
    saveWorkspace(std::make_shared<Workspace>(result.getName(), result.getPath()));
  }
}

void ConfigService::closeWorkspace() {
  currentWorkspace = nullptr;
}

bool ConfigService::isEditingWorkspace() {
  return currentWorkspace != nullptr;
}

json ConfigService::currentConfig()
{
  json config;
  config[MidiJsonKey] = MidiService::getService()->config();
  config[OscJsonKey] = OscillationService::getService()->config();
  config[ShadersJsonKey] = ShaderChainerService::getService()->config();
  config[LayoutJsonKey] = LayoutStateService::getService()->config();
  config[SourcesJsonKey] = VideoSourceService::getService()->config();
  config[ConnectionsJsonKey] = ShaderChainerService::getService()->connectionsConfig();
  config[StrandsJsonKey] = StrandService::getService()->config();
  config[ParametersJsonKey] = ParameterService::getService()->config();
  config[AudioJsonKey] = AudioSourceService::getService()->config();
  if (currentWorkspace != nullptr) {
    config[WorkspaceJsonKey] = currentWorkspace->serialize();
  }
  config[ConfigTypeKey] = ConfigTypeFull;
  return config;
}

void ConfigService::saveConfigFile(std::string path)
{
  if (isLoading) return;
  
  json config = currentConfig();
  
  if (lastConfig == config) {
    log("No need to save identical config");
    return;
  }
  
  std::ofstream fileStream;
  fileStream.open(path.c_str(), std::ios::trunc);
  
  if (fileStream.is_open())
  {
    //    std::cout << config.dump(4) << std::endl;
    fileStream << config.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore);
    fileStream.close();
    
    std::cout << "Successfully saved config" << std::endl;
    lastConfig = config;
  }
  else
  {
    //    std::cout << config.dump(4) << std::endl;
    log("Problem saving config.");
  }
}

void ConfigService::saveWorkspace(std::shared_ptr<Workspace> workspace) {
  currentWorkspace = workspace;
  saveConfigFile(workspace->path);
  updateWorkspace(workspace);
}

void ConfigService::loadWorkspace(std::shared_ptr<Workspace> workspace) {
  currentWorkspace = workspace;
  loadConfigFile(workspace->path);
  updateWorkspace(workspace);
}

void ConfigService::updateWorkspace(std::shared_ptr<Workspace> workspace) {
  ofSetWindowTitle(workspace->name);
}

bool ConfigService::loadWorkspaceDialogue() {
  // Present a file dialog to load the config file
  ofFileDialogResult result = ofSystemLoadDialog("Open Workspace", false);
  if (result.bSuccess)
  {
    loadWorkspace(std::make_shared<Workspace>(result.getName(), result.getPath()));
  }
  return result.bSuccess;
}


std::vector<std::string> ConfigService::loadStrandFile(std::string path)
{
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  json data;
  
  
  if (!fileStream.is_open())
  {
      std::cerr << "Error opening file: " << path << "\n";
      std::cerr << "Error code: " << errno << " (" << std::strerror(errno) << ")\n";
      return {};
  }
  
  if (fileStream.is_open())
  {
    try
    {
      fileStream >> data;
    }
    catch (int code)
    {
      log("Could not load chainer file for %s.", path.c_str());
      return {};
    }
  }
  
  std::vector<std::string> ids;
  
  if (data[ShadersJsonKey].is_object())
  {
    std::vector<std::string> shaders = ShaderChainerService::getService()->idsFromLoadingConfig(data[ShadersJsonKey]);
    ids.insert(ids.end(), shaders.begin(), shaders.end());
  }
  
  if (data[SourcesJsonKey].is_object())
  {
    std::vector<std::string> sources = VideoSourceService::getService()->idsFromLoadingConfig(data[SourcesJsonKey]);
    ids.insert(ids.end(), sources.begin(), sources.end());
  }
  
  if (data[ConnectionsJsonKey].is_object())
  {
    ShaderChainerService::getService()->loadConnectionsConfig(data[ConnectionsJsonKey]);
  }
  
  if (data[ParametersJsonKey].is_object())
  {
    ParameterService::getService()->loadConfig(data[ParametersJsonKey]);
  }
  return ids;
}

void ConfigService::loadConfigFile(std::string path)
{
  isLoading = true;
  std::fstream fileStream;
  fileStream.open(path, std::ios::in);
  json data;
  
  if (fileStream.is_open())
  {
    try
    {
      // Make sure the file isn't empty
      if (fileStream.peek() == std::ifstream::traits_type::eof())
      {
        log("JSON file for %s is empty.", path.c_str());
        isLoading = false;
        return;
      }
      fileStream >> data;
    }
    catch (int code)
    {
      log("Could not load JSON file for %s.", path.c_str());
      isLoading = false;
      return;
    }
  }
  
  //  std::cout << data.dump(4) << std::endl;
  
  if (!data.is_object())
  {
    log("Failed to parse JSON file.");
    isLoading = false;
    return;
  }
  
  // Load Layout State
  if (data[LayoutJsonKey].is_object())
  {
    try {
      LayoutStateService::getService()->loadConfig(data[LayoutJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Layout config: " << e.what() << std::endl;
    }
  }
  
  // Load MIDI Service Config
  if (data[MidiJsonKey].is_object())
  {
    try {
      MidiService::getService()->loadConfig(data[MidiJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading MIDI config: " << e.what() << std::endl;
    }
  }
  
  // Load Video Source Service Config
  if (data[SourcesJsonKey].is_object())
  {
    try {
      VideoSourceService::getService()->clear();
      VideoSourceService::getService()->loadConfig(data[SourcesJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Sources config: " << e.what() << std::endl;
    }
  }
  
  // Load Shader Chainer Service Config
  if (data[ShadersJsonKey].is_object())
  {
    try {
      ShaderChainerService::getService()->clear();
      ShaderChainerService::getService()->loadConfig(data[ShadersJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Shaders config: " << e.what() << std::endl;
    }
  }
  
  // Load Connections Config
  if (data[ConnectionsJsonKey].is_object())
  {
    try {
      ShaderChainerService::getService()->loadConnectionsConfig(data[ConnectionsJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Connections config: " << e.what() << std::endl;
    }
  }
  
  // Load Oscillation Service Config
  if (data[OscJsonKey].is_object())
  {
    try {
      OscillationService::getService()->loadConfig(data[OscJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading OSC config: " << e.what() << std::endl;
    }
  }
  
  // Load Strand Service Config
  if (data[StrandsJsonKey].is_object())
  {
    try {
      StrandService::getService()->loadConfig(data[StrandsJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Strands config: " << e.what() << std::endl;
    }
  }
  
  // Load Parameter Service Config
  if (data[ParametersJsonKey].is_object())
  {
    try {
      ParameterService::getService()->loadConfig(data[ParametersJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Parameters config: " << e.what() << std::endl;
    }
  }
  
  // Load Audio Source Service Config
  if (data[AudioJsonKey].is_object())
  {
    try {
      AudioSourceService::getService()->loadConfig(data[AudioJsonKey]);
    } catch (const json::exception& e) {
      std::cerr << "Error loading Audio config: " << e.what() << std::endl;
    }
  }
  
  // Load Workspace Config
  if (data[WorkspaceJsonKey].is_object()) {
    try {
      std::string workspaceName = data[WorkspaceJsonKey]["name"];
      std::string workspacePath = data[WorkspaceJsonKey]["path"];
      loadWorkspace(std::make_shared<Workspace>(workspaceName, workspacePath));
    } catch (const json::exception& e) {
        std::cerr << "Error loading Workspace config: " << e.what() << std::endl;
    }
  }
    
  isLoading = false;
}

