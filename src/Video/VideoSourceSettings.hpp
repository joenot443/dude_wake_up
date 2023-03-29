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
  
  VideoSourceSettings(std::string sourceId, json j) :
  width(std::make_shared<Parameter>("width", sourceId, 720, 100, 2000)),
  height(std::make_shared<Parameter>("height", sourceId, 480, 100, 2000)),
  resolution(std::make_shared<Parameter>("resolution", sourceId, 3, 0, 5)),
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
