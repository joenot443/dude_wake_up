//
//  ShaderChainerService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef ShaderChainerService_hpp
#define ShaderChainerService_hpp

#include "AvailableShader.hpp"
#include "ConfigurableService.hpp"
#include "Shader.hpp"
#include "AvailableShaderChainer.hpp"
#include "ShaderChainer.hpp"
#include "observable.hpp"
#include <stdio.h>

using json = nlohmann::json;

// Singleton service for managing ShaderChainer objects
class ShaderChainerService : public ConfigurableService {

private:
  
  // Maps a shaderChainerId to a ShaderChainer
  std::map<std::string, std::shared_ptr<ShaderChainer>> shaderChainerMap;
  
  // Maps a shaderId to a Shader
  std::map<std::string, std::shared_ptr<Shader>> shadersMap;
  
  // Maps a Shader to its parent ShaderChainer
  std::map<std::string, std::shared_ptr<ShaderChainer>>
      shaderIdShaderChainerMap;
  
  // Maps a VideoSource to a vector of ShaderChainers associated with that VideoSource
  std::map<std::string, std::vector<std::shared_ptr<ShaderChainer>>>
      videoSourceIdShaderChainerMap;

  observable::subject<void()> shaderChainerUpdateSubject;

public:
  static ShaderChainerService *service;
  ShaderChainerService(){};
  void setup();

  static ShaderChainerService *getService() {
    if (!service) {
      service = new ShaderChainerService;
      service->setup();
    }
    return service;
  }
  
  static void recreateService() {
    service = new ShaderChainerService;
    service->setup();
  }

  // ShaderChainer Ops
  std::shared_ptr<ShaderChainer> selectedShaderChainer;
  std::shared_ptr<ShaderChainer> addNewShaderChainer(std::shared_ptr<VideoSource> videoSource);
  void addShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void removeShaderChainer(std::string id);
  void removeShaderChainerReferenceFromSourceMap(std::shared_ptr<ShaderChainer> shaderChainer);
  void selectShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void subscribeToShaderChainerUpdates(std::function<void()> callback);
  void associateShaderWithChainer(std::string shaderId,
                                  std::string shaderChainerId);
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers();
  std::vector<std::string> shaderChainerNames();
  void updateShaderChainers();
  std::shared_ptr<Shader> selectedShader;
  
  // Accessing
  
  // Returns the ShaderChainer for that chainerId
  std::shared_ptr<ShaderChainer> shaderChainerForId(std::string id);
  
  // Returns the ShaderChainer for that shaderId
  std::shared_ptr<ShaderChainer> shaderChainerForShaderId(std::string id);
  
  // Returns the ShaderChainer for that videoSourceId which also has a frontAux
  // which corresponds to the passed shaderId.
  std::shared_ptr<ShaderChainer> shaderChainerForAuxShaderIdAndVideoSourceId(std::string auxShaderId, std::string videoSourceId);
  
  // Returns the ShaderChainers which are sourced by that VideoSourceId
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainersForVideoSourceId(std::string id);
  
  // Returns the first ShaderChainer sourced by that VideoSourceId.
  std::shared_ptr<ShaderChainer> implicitShaderChainerForVideoSourceId(std::string id);
  
  // Returns true if the implicit ShaderChainer does not have a front or frontAux.
  bool implicitShaderChainerForVideoSourceIdIsEmpty(std::string id);
  
  // Modifying
  void selectShader(std::shared_ptr<Shader> shader);
  void addShader(std::shared_ptr<Shader> shader);
  std::shared_ptr<Shader> makeShader(ShaderType type);
  void removeShader(std::shared_ptr<Shader> shader);
  // Breaks the Shader's link to its next Shader
  void breakShaderNextLink(std::shared_ptr<Shader> shader);
  // Break the link from a Shader to the Aux Input of another
  void breakShaderAuxLink(std::shared_ptr<Shader> startShader,
                          std::shared_ptr<Shader> endShader);
  // Break the link from a Source to the Aux Input of a Shader
  void breakSourceShaderAuxLink(std::shared_ptr<VideoSource> source,
                          std::shared_ptr<Shader> shader);
  
  // Breaks the ShaderChainer's link to its front Shader
  void breakShaderChainerFront(std::shared_ptr<ShaderChainer> shaderChainer);
  
  // Removes the ShaderChainers associated with that VideoSource
  void removeShaderChainersForVideoSourceId(std::string id);

  // Links the Shader to the next Shader
  void linkShaderToNext(std::shared_ptr<Shader> sourceShader,
                        std::shared_ptr<Shader> destShader);
  
  // Links the VideoSource to a Shader's Aux
  std::shared_ptr<ShaderChainer> linkVideoSourceToShaderAux(std::shared_ptr<VideoSource> auxSource,
                    std::shared_ptr<Shader> destShader);
  
  // Sets an Aux Shader for a Shader
  void setAuxShader(std::shared_ptr<Shader> auxShader,
                    std::shared_ptr<Shader> destShader);
  
  // Sets a Shader to be the front of a ShaderChainer
  void addShaderToFront(std::shared_ptr<Shader> destShader,
                        std::shared_ptr<ShaderChainer> chainer);
  
  // Link the VideoSource to a Shader.
  // This will either create a new ShaderChainer, or will set the `front`
  // Shader for an existing ShaderChainer using that source.
  std::shared_ptr<ShaderChainer> linkVideoSourceToShader(std::shared_ptr<VideoSource> source,
                               std::shared_ptr<Shader> shader);
  
  
  std::shared_ptr<Shader> shaderForId(std::string id);
  std::vector<std::shared_ptr<Shader>> shaders();

  // Available Shaders
  std::vector<std::shared_ptr<AvailableShader>> availableBasicShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableMixShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableTransformShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableFilterShaders;

  // Constructor Ops
  std::shared_ptr<Shader> shaderForType(ShaderType type, std::string shaderId,
                                        json j);

  void clear();
  json config() override;
  void loadConfig(json j) override;
};

#endif /* ShaderChainerService_hpp */
