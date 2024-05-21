//
//  ParameterService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-24.
//

#ifndef ParameterService_h
#define ParameterService_h

#include <stdio.h>
#include "ShaderType.hpp"
#include "json.hpp"
#include "ofxMidi.h"
#include "observable.hpp"
#include "ofMain.h"
#include "MidiPairing.hpp"
#include "Parameter.hpp"
#include "FavoriteParameter.hpp"
#include "ConfigurableService.hpp"

using json = nlohmann::json;

static const std::string FavoriteParameterJsonKey = "favoriteParameters";
static const std::string FavoriteShadersJsonKey = "favoriteShaders";
static const std::string StageShadersJsonKey = "stageShaders";

class ParameterService: public ConfigurableService {
private:
  observable::subject<void()> configUpdateSubject;
public:
  void clear();
  
  std::shared_ptr<Parameter> parameterForId(std::string paramId);
  static ParameterService* service;
  
  void registerParameter(std::shared_ptr<Parameter> parameter);
  
  void tickParameters();
  
  std::shared_ptr<FavoriteParameter> addFavoriteParameter(std::shared_ptr<Parameter> parameter);
  void removeFavoriteParameter(std::shared_ptr<Parameter> parameter);
  bool hasFavoriteParameterFor(std::shared_ptr<Parameter> parameter);
  
  std::vector<std::shared_ptr<FavoriteParameter>> favoriteParameters();
  
  std::map<std::string, std::shared_ptr<FavoriteParameter>> favoriteParameterMap;
  std::map<std::string, std::shared_ptr<Parameter>> parameterMap;
  
  std::shared_ptr<Parameter> selectedParameter;
  
  void notifyFavoritesUpdate();
  void subscribeToFavoritesUpdates(std::function<void()> callback);
  std::set<ShaderType> favoriteShaderTypes;
  void addFavoriteShaderType(ShaderType type);
  bool isShaderTypeFavorited(ShaderType type);
  void removeFavoriteShaderType(ShaderType type);
  void toggleFavoriteShaderType(ShaderType type);
  
  std::set<std::string> stageShaderIds;
  // If no stageShaderIds have been set otherwise, use the defaultStageShaderId
  // which maps to the Shader with the most traversals (furthest along).
  std::pair<std::string, int> defaultStageShaderIdDepth;
  void addStageShaderId(std::string shaderId);
  bool isShaderIdStage(std::string shaderId);
  void removeStageShaderId(std::string shaderId);
  void toggleStageShaderId(std::string shaderId);
  
  void loadConfig(json j);
  json config();
  
  ParameterService() {};
  static ParameterService* getService() {
    if (!service) {
      service = new ParameterService;
    }
    return service;
    
  }
};

#endif /* ParameterService_h */
