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
static const std::string FavoriteSourcesJsonKey = "favoriteSources";
static const std::string StageShadersJsonKey = "stageShaders";

class ParameterService: public ConfigurableService {
private:
  observable::subject<void()> configUpdateSubject;
public:
  void clear();
  
  std::shared_ptr<Parameter> parameterForId(std::string paramId);
  static ParameterService* service;
  
  void registerParameter(std::shared_ptr<Parameter> parameter);

  // Drop the given parameters from parameterMap, drivenParameters, and the
  // recentlyActiveParameters list. Without this the map grows unbounded —
  // every Shader/Source registers parameters but nothing ever erased them.
  void unregisterParameters(const std::vector<std::shared_ptr<Parameter>>& parameters);

  void tickParameters();

  // Hot-list maintenance. tickParameters does no work beyond walking these
  // small lists, so callers MUST notify via these helpers rather than poking
  // Parameter::driver / Parameter::active directly.
  void trackDriver(std::shared_ptr<Parameter> parameter);
  void untrackDriver(std::shared_ptr<Parameter> parameter);
  void markActive(std::shared_ptr<Parameter> parameter);

  std::vector<std::shared_ptr<Parameter>> allActiveParameters();
  
  std::shared_ptr<FavoriteParameter> addFavoriteParameter(std::shared_ptr<Parameter> parameter);
  void removeFavoriteParameter(std::shared_ptr<Parameter> parameter);
  bool hasFavoriteParameterFor(std::shared_ptr<Parameter> parameter);
  
  std::vector<std::shared_ptr<FavoriteParameter>> favoriteParameters();
  
  std::map<std::string, std::shared_ptr<FavoriteParameter>> favoriteParameterMap;
  std::map<std::string, std::shared_ptr<Parameter>> parameterMap;

  // Hot lists. These are subsets of parameterMap; tickParameters iterates
  // only these. Maintained by trackDriver/untrackDriver/markActive.
  std::vector<std::shared_ptr<Parameter>> drivenParameters;
  std::vector<std::shared_ptr<Parameter>> recentlyActiveParameters;
  
  std::shared_ptr<Parameter> selectedParameter;
  
  void notifyFavoritesUpdate();
  void subscribeToFavoritesUpdates(std::function<void()> callback);
  std::set<ShaderType> favoriteShaderTypes;
  void addFavoriteShaderType(ShaderType type);
  bool isShaderTypeFavorited(ShaderType type);
  void removeFavoriteShaderType(ShaderType type);
  void toggleFavoriteShaderType(ShaderType type);
  
  std::set<int> favoriteSourceTypes;
  void toggleFavoriteSourceType(int sourceType);
  bool isFavoriteSourceType(int sourceType);

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
  void setup();
  static ParameterService* getService() {
    if (!service) {
      service = new ParameterService;
    }
    return service;
    
  }
};

#endif /* ParameterService_h */
