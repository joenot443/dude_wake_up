//
//  ConfigService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#ifndef ConfigService_hpp
#define ConfigService_hpp

#include <stdio.h>

#include "ShaderChainer.hpp"

using json = nlohmann::json;

class ConfigService {
private:
  
public:
  
  // Shaders
  
  void saveShaderChainerConfigFile(ShaderChainer* chainer, std::string path);
  ShaderChainer* loadShaderChainerConfigFile(std::string name);
  
  json jsonFromParameters(std::vector<Parameter*> parameters);

  void saveDefaultConfigFile();
  void loadDefaultConfigFile();
  
  void saveConfigFile(std::string path);
  void loadConfigFile(std::string path);
  
  static ConfigService* service;
  ConfigService() {};
  static ConfigService* getService() {
    if (!service) {
      service = new ConfigService;
    }
    return service;
    
  }
};

#endif /* ConfigService_hpp */
