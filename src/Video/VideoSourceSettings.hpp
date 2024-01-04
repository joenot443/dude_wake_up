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
  std::shared_ptr<Parameter> resolution;
  std::shared_ptr<Parameter> maskColor;
  std::shared_ptr<Parameter> maskEnabled;
  std::shared_ptr<Parameter> maskTolerance;
  
  VideoSourceSettings() :
  width(std::make_shared<Parameter>("width", 720, 100, 2000)),
  height(std::make_shared<Parameter>("height", 480, 100, 2000)),
  resolution(std::make_shared<Parameter>("resolution", 3, 0, 5)),
  maskColor(std::make_shared<Parameter>("maskColor", 3, 0, 5)),
  maskEnabled(std::make_shared<Parameter>("maskEnabled", 0, 0, 1)),
  maskTolerance(std::make_shared<Parameter>("maskTolerance", 0.1, 0, 1)),
  Settings() {
    parameters = {width, height};
    updateResolutionSettings();
  }
  
  VideoSourceSettings(std::string sourceId, json j) :
  width(std::make_shared<Parameter>("width", 720, 100, 2000)),
  height(std::make_shared<Parameter>("height", 480, 100, 2000)),
  resolution(std::make_shared<Parameter>("resolution", 3, 0, 5)),
  maskColor(std::make_shared<Parameter>("Mask Color", 3, 0, 5)),
  maskEnabled(std::make_shared<Parameter>("Mask Enabled", 0, 0, 1)),
  maskTolerance(std::make_shared<Parameter>("Mask Tolerance", 0.1, 0, 1)),
  Settings() {
    parameters = {width, height};
    updateResolutionSettings();
    load(j);
  }
  
  void updateResolutionSettings() {
    auto i = resolution->intValue;
    if (i == 0) {
      width->setValue(426.0f);
      height->setValue(240.0f);
    }
    else if (i == 1) {
      width->setValue(640.0f);
      height->setValue(360.0f);
    }
    else if (i == 2) {
      width->setValue(854.0f);
      height->setValue(480.0f);
    }
    else if (i == 3) {
      width->setValue(1280.0f);
      height->setValue(720.0f);
    }
    else if (i == 4) {
      width->setValue(1920.0f);
      height->setValue(1080.0f);
    }
    else if (i == 5) {
      width->setValue(2560.0f);
      height->setValue(1440.0f);
    }
    else if (i == 6) {
      width->setValue(3840.0f);
      height->setValue(2160.0f);
    }
  }
};

#endif /* VideoSourceSettings_h */
