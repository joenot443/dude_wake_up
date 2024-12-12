//
//  IconService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#ifndef IconService_hpp
#define IconService_hpp

#include <stdio.h>
#include "Icon.hpp"
#include "ofMain.h"

class IconService
{
private:
  std::vector<std::shared_ptr<Icon>> icons;

public:
  void setup();
  void populateIcons();
  std::vector<std::shared_ptr<Icon>> availableIcons();
  std::vector<std::string> availableIconNames();
  std::shared_ptr<Icon> defaultIcon();
  std::shared_ptr<Icon> iconWithName(std::string name);
  std::shared_ptr<ofShader> solidColorShader;
  
  
  
  static IconService *service;
  IconService() : solidColorShader(std::make_shared<ofShader>()) {};
  
  
  static IconService *getService() {
    if (!service) {
      service = new IconService;
      service->setup();
    }
    return service;
  }
};

#endif /* IconService_hpp */
