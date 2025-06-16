//
//  JSONSerializable.h
//  dude_wake_up
//
//  Created by Joe Crozier on 10/24/22.
//

#ifndef JSONSerializable_h
#define JSONSerializable_h

#include "json.hpp"

using json = nlohmann::json;

class JSONSerializable {
public:
  void load(json j);
  json serialize();

};


#endif /* JSONSerializable_h */
