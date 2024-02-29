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
  return ofFilePath::join(ofFilePath::getUserHomeDir(), "/nottawa");
}

std::string ConfigService::libraryFolderFilePath()
{
  return relativeFilePathWithinNottawaFolder("/videos/");
}

std::string ConfigService::relativeFilePathWithinNottawaFolder(std::string filePath)
{
  return ofFilePath::join(nottawaFolderFilePath(), filePath);
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
  return ofFilePath::join(nottawaFolderFilePath(), formatString("/shaders/%s", shaderTypeName(type).c_str()));
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
  directory.open(shaderConfigFolderForType(type));
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
    ;
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

void ConfigService::loadDefaultConfigFile()
{
  const char *homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  loadConfigFile(path);
}

void ConfigService::saveDefaultConfigFile()
{
  
  const char *homeDir = getenv("HOME");
  auto path = formatString("%s/config.json", homeDir);
  saveConfigFile(path);
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
  config[ConfigTypeKey] = ConfigTypeFull;
  return config;
}

void ConfigService::saveConfigFile(std::string path)
{
  if (isLoading) return;
  
  json config = currentConfig();

  std::ofstream fileStream;
  fileStream.open(path.c_str(), std::ios::trunc);

  if (fileStream.is_open())
  {
    //    std::cout << config.dump(4) << std::endl;
    fileStream << config.dump(4);
    fileStream.close();

    std::cout << "Successfully saved config" << std::endl;
  }
  else
  {
    //    std::cout << config.dump(4) << std::endl;
    log("Problem saving config.");
  }
}

std::vector<std::string> ConfigService::loadStrandFile(std::string path)
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
        return 0;
      }
      fileStream >> data;
    }
    catch (int code)
    {
      log("Could not load JSON file for %s.", path.c_str());
      return;
    }
  }

//  std::cout << data.dump(4) << std::endl;

  if (!data.is_object())
  {
    log("Failed to parse JSON file.");
  }
  try {
    
    
    if (data[LayoutJsonKey].is_object())
    {
      LayoutStateService::getService()->loadConfig(data[LayoutJsonKey]);
    }
    
    if (data[MidiJsonKey].is_object())
    {
      MidiService::getService()->loadConfig(data[MidiJsonKey]);
    }
    
    if (data[OscJsonKey].is_object())
    {
      OscillationService::getService()->loadConfig(data[OscJsonKey]);
    }
    
    if (data[SourcesJsonKey].is_object())
    {
      VideoSourceService::getService()->clear();
      VideoSourceService::getService()->loadConfig(data[SourcesJsonKey]);
    }
    
    if (data[ShadersJsonKey].is_object())
    {
      ShaderChainerService::getService()->clear();
      ShaderChainerService::getService()->loadConfig(data[ShadersJsonKey]);
    }
    
    if (data[ConnectionsJsonKey].is_object())
    {
      ShaderChainerService::getService()->loadConnectionsConfig(data[ConnectionsJsonKey]);
    }

    if (data[StrandsJsonKey].is_object())
    {
      StrandService::getService()->loadConfig(data[StrandsJsonKey]);
    }

  } catch (const std::exception& e) {
    std::cerr << "Error reading or parsing config file: " << e.what() << std::endl;
  }
  isLoading = false;
}
