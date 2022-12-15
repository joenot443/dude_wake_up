//
//  ShaderChainerService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef ShaderChainerService_hpp
#define ShaderChainerService_hpp

#include <stdio.h>
#include "ShaderChainer.hpp"
#include "Shader.hpp"
#include "ConfigurableService.hpp"

using json = nlohmann::json;

// Singleton service for managing ShaderChainer objects
class ShaderChainerService: public ConfigurableService {

private:
  std::map<std::string, std::shared_ptr<ShaderChainer>> shaderChainerMap;

public:
  static ShaderChainerService* service;
  ShaderChainerService() {};

  static ShaderChainerService* getService() {
    if (!service) {
      service = new ShaderChainerService;
    }
    return service;
  }

  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers();
  std::vector<std::string> shaderChainerNames();
  void updateShaderChainers();
  void removeShaderChainer(std::string id);
  void addShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void selectShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void selectShader(std::shared_ptr<Shader> shader);
  int count();
  std::shared_ptr<ShaderChainer> shaderChainerForId(std::string id);
  std::shared_ptr<Shader> selectedShader;
  std::shared_ptr<ShaderChainer> selectedShaderChainer;
  
  json config() override;
  void loadConfig(json j) override;
};


#endif /* ShaderChainerService_hpp */
