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

// Singleton service for managing ShaderChainer objects

class ShaderChainerService {

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
  std::shared_ptr<ShaderChainer> shaderChainerForId(std::string id);
};


#endif /* ShaderChainerService_hpp */
