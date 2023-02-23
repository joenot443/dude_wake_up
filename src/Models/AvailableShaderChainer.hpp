//
//  AvailableShaderChainer.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/5/23.
//

#ifndef AvailableShaderChainer_h
#define AvailableShaderChainer_h
#include "json.hpp"

using json = nlohmann::json;

struct AvailableShaderChainer {
  std::shared_ptr<std::string> chainerName;
  std::shared_ptr<std::string> path;

  AvailableShaderChainer(std::string chainerName, std::string path)
      : chainerName(std::make_shared<std::string>(chainerName)), path(std::make_shared<std::string>(path)) {};
};

#endif /* AvailableShaderChainer_h */
