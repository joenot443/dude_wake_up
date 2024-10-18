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
  std::shared_ptr<Parameter> maskColor;
  std::shared_ptr<Parameter> maskEnabled;
  
  // 0: Color, 1: Luma
  std::shared_ptr<Parameter> maskType;
  std::shared_ptr<Parameter> maskTolerance;

  
  
  // Invert
  std::shared_ptr<Parameter> invert;
  
  std::shared_ptr<Parameter> start;
  std::shared_ptr<Parameter> end;
  
  VideoSourceSettings(std::string sourceId, json j) :
  width(std::make_shared<Parameter>("width", 720, 100, 2000)),
  height(std::make_shared<Parameter>("height", 480, 100, 2000)),
  maskColor(std::make_shared<Parameter>("Mask Color", 3, 0, 5)),
  maskEnabled(std::make_shared<Parameter>("Mask Enabled", 0, 0, 1)),
  maskTolerance(std::make_shared<Parameter>("Tolerance", 0.05, 0, 1)),
  invert(std::make_shared<Parameter>("Invert", 0.0, 1.0, 1.0)),
  start(std::make_shared<Parameter>("start", 0, 0, 1)),
  end(std::make_shared<Parameter>("end", 1, 0, 1)),
  Settings() {
    parameters = {width, height, maskColor, maskEnabled, maskTolerance, start, end, invert};
    load(j);
  }
};

#endif /* VideoSourceSettings_h */
