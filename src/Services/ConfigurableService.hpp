//
//  ConfigurableService.h
//  dude_wake_up
//
//  Created by Joe Crozier on 12/8/22.
//

#ifndef ConfigurableService_h
#define ConfigurableService_h

#include <stdio.h>
#include "ofMain.h"
#include "json.hpp"

using json = nlohmann::json;

class ConfigurableService {
public:
  virtual json config() = 0;
  virtual void loadConfig(json j) = 0;
};

#endif /* ConfigurableService_h */
