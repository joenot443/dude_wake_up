//
//  Workspace.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 5/7/24.
//

#ifndef Workspace_h
#define Workspace_h

#include "ofMain.h"

struct Workspace {
  std::string name;
  std::string path;
  
  Workspace(std::string name, std::string path) : name(name), path(path) {}
};

#endif /* Workspace_h */
