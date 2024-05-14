//
//  Connection.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/22/23.
//

#include <stdio.h>
#include "Connection.hpp"

json Connection::serialize() {  
  json j;
  j["id"] = id;
  j["start"] = start->connId();
  j["end"] = end->connId();
  j["type"] = type;
  j["slot"] = inputSlot;
  return j;
}
