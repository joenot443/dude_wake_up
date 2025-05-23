//
//  Workspace.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 5/7/24.
//

#ifndef Workspace_h
#define Workspace_h

#include "json.hpp"
#include "JSONSerializable.hpp"
struct Workspace : public JSONSerializable {
  std::string name;
  std::string path;
  
  Workspace(std::string name, std::string path) : name(name), path(path) {}

  json serialize()  {
    json j;
    j["name"] = name;
    j["path"] = path;
    return j;
  }

  void load(json j)  {
    name = j["name"];
    path = j["path"];
  }
};

#endif /* Workspace_h */
