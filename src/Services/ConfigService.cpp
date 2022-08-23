//
//  ConfigService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/7/22.
//

#include "ConfigService.hpp"
#include "ParameterService.h"
#include "json.hpp"
#include "Console.hpp"
#include <ostream>
#include <fstream>

json jsonFromParameters(std::vector<Parameter*> parameters) {
  json j;
  for (auto p : parameters) {
    j[p->paramId.c_str()] = p->value;
  }
}

void ConfigService::saveConfigFile(VideoSettings* settings, std::string path) {
  std::ofstream fileStream;
  fileStream.open(path.c_str(), std::ios::trunc);
  json container;
  container = settings->serialize(container);
  
  if (fileStream.is_open()) {
    std::cout << container.dump(4) << std::endl;
    fileStream << container.dump(4);
    fileStream.close();
  } else {
    log("Problem saving config.");
  }
}


VideoSettings* ConfigService::loadConfigFile(std::string name) {
  std::fstream fileStream;
  fileStream.open(name, std::ios::in);
  if (fileStream.is_open()) {
    json data;
    try {
      fileStream >> data;
    }
    catch (int code) {
      log("Failed to load JSON file.");
      return;
    }
    ParameterService::getService()->loadParameters(&data);
    log("Loaded config file.");
  }
}

