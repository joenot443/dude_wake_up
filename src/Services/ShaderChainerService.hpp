//
//  ShaderChainerService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef ShaderChainerService_hpp
#define ShaderChainerService_hpp

#include "ConfigurableService.hpp"
#include "AvailableShader.hpp"
#include "Shader.hpp"
#include "ShaderChainer.hpp"
#include "observable.hpp"
#include <stdio.h>

using json = nlohmann::json;

// Singleton service for managing ShaderChainer objects
class ShaderChainerService : public ConfigurableService {

private:
  std::map<std::string, std::shared_ptr<ShaderChainer>> shaderChainerMap;
  std::map<std::string, std::shared_ptr<Shader>> shadersMap;
  // Maps a Shader to its parent ShaderChainer
  std::map<std::string, std::shared_ptr<ShaderChainer>>
      shaderIdShaderChainerMap;

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

  // ShaderChainer Ops
  std::shared_ptr<ShaderChainer> selectedShaderChainer;
  void addNewShaderChainer(std::shared_ptr<VideoSource> videoSource);
  void addShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void removeShaderChainer(std::string id);
  void selectShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void subscribeToShaderChainerUpdates(std::function<void()> callback);
  void associateShaderWithChainer(std::string shaderId,
                                  std::shared_ptr<ShaderChainer> chainer);
  std::shared_ptr<ShaderChainer> shaderChainerForId(std::string id);
  std::shared_ptr<ShaderChainer> shaderChainerForShaderId(std::string id);
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers();
  std::vector<std::string> shaderChainerNames();
  void updateShaderChainers();

  // Shader Ops
  std::shared_ptr<Shader> selectedShader;
  void selectShader(std::shared_ptr<Shader> shader);
  void addShader(std::shared_ptr<Shader> shader);
  std::shared_ptr<Shader> makeShader(ShaderType type);
  void removeShader(std::shared_ptr<Shader> shader);
  // Breaks the Shader's link to its next Shader
  void breakShaderNextLink(std::shared_ptr<Shader> shader);
  // Break the link from a Shader to the Aux Input of another
  void breakShaderAuxLink(std::shared_ptr<Shader> startShader,
                          std::shared_ptr<Shader> endShader);
  // Breaks the ShaderChainer's link to its front Shader
  void breakShaderChainerFront(std::shared_ptr<ShaderChainer> shaderChainer);

  // Links the Shader to the next Shader
  void linkShaderToNext(std::shared_ptr<Shader> sourceShader,
                        std::shared_ptr<Shader> destShader);
  // Sets an Aux Shader for a Shader
  void setAuxShader(std::shared_ptr<Shader> auxShader,
                    std::shared_ptr<Shader> destShader);
  // Sets a Shader to be the front of a ShaderChainer
  void addShaderToFront(std::shared_ptr<Shader> destShader,
                        std::shared_ptr<ShaderChainer> chainer);

  std::shared_ptr<Shader> shaderForId(std::string id);
  std::vector<std::shared_ptr<Shader>> shaders();

  // Available Shaders
  std::vector<std::shared_ptr<AvailableShader>> availableShaders;

  // Constructor Ops
  std::shared_ptr<Shader> shaderForType(ShaderType type, std::string shaderId,
                                        json j);

  json config() override;
  void loadConfig(json j) override;
};

#endif /* ShaderChainerService_hpp */
