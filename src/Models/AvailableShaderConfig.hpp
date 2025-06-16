//
//  AvailableShaderConfig.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/4/23.
//

#ifndef AvailableShaderConfig_h
#define AvailableShaderConfig_h

#include "json.hpp"

using json = nlohmann::json;

struct AvailableShaderConfig {
  std::string name;
  std::string path;

  AvailableShaderConfig(std::string name, std::string path)
      : name(std::string(name)), path(std::string(path)) {};
};

#endif /* AvailableShaderConfig_h */
