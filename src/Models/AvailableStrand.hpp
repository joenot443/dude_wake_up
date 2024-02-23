//
//  AvailableStrand.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/5/23.
//

#ifndef AvailableStrand_h
#define AvailableStrand_h
#include "json.hpp"

using json = nlohmann::json;

struct AvailableStrand {
  std::string name;
  std::string path;

  AvailableStrand(std::string name, std::string path)
      : name(name), path(path) {};
};

#endif /* AvailableStrand_h */
