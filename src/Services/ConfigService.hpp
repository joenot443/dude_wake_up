//
//  ConfigService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#ifndef ConfigService_hpp
#define ConfigService_hpp

#include <stdio.h>
#include "VideoSettings.h"

using json = nlohmann::json;

class ConfigService {
private:
  
public:
  void saveConfigFile(VideoSettings* settings, std::string path);
  VideoSettings* loadConfigFile(std::string name);
  json jsonFromParameters(std::vector<Parameter*> parameters);
  
  
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
