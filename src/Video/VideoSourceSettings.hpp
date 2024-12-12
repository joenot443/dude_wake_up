//
//  VideoSourceSettings.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/16/23.
//

#ifndef VideoSourceSettings_h
#define VideoSourceSettings_h

#include "ofMain.h"
#include "Parameter.hpp"
#include "Settings.hpp"

class VideoSourceSettings: public Settings {
public:
  std::shared_ptr<Parameter> width;
  std::shared_ptr<Parameter> height;
  
  std::shared_ptr<Parameter> start;
  std::shared_ptr<Parameter> end;

  std::shared_ptr<Parameter> selectorParam;
  
  VideoSourceSettings(std::string sourceId, json j) :
  width(std::make_shared<Parameter>("width", 720, 100, 2000)),
  height(std::make_shared<Parameter>("height", 480, 100, 2000)),
  start(std::make_shared<Parameter>("start", 0, 0, 1)),
  end(std::make_shared<Parameter>("end", 1, 0, 1)),
  selectorParam(std::make_shared<Parameter>("Selector", ParameterType_Int)),
  Settings() {
    parameters = {width, height, start, end};
    load(j);
  }
};

#endif /* VideoSourceSettings_h */
