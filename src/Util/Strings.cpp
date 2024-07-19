//
//  Strings.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/26/24.
//

#include <stdio.h>
#include "Strings.hpp"
#include <fstream>
#include "ofMain.h"

nlohmann::json StringManager::stringJson;

void StringManager::loadStrings() {
  ofFile file = ofFile("misc/strings.json");
  file >> stringJson;
}

std::string StringManager::get(const std::string& key) {
    return stringJson.value(key, "Undefined");
}
