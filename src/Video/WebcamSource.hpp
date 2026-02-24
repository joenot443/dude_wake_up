//
//  WebcamSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef WebcamSource_hpp
#define WebcamSource_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"

using json = nlohmann::json;

class WebcamVideoSourceSettings : public VideoSourceSettings {
public:
    std::shared_ptr<Parameter> deviceId;

    WebcamVideoSourceSettings(std::string sourceId, json j)
  : VideoSourceSettings(sourceId, j), deviceId(std::make_shared<Parameter>("Device", 0.0, 0.0, 100.0, ParameterType_Int)) {
    parameters.push_back(deviceId);
    registerParameters();
  }
};

class WebcamSource : public VideoSource {
  
public:
  WebcamSource(std::string id, std::string name) : VideoSource(id, name, VideoSource_webcam), settings(std::make_shared<WebcamVideoSourceSettings>(id, 0)) {};
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  ofShader maskShader;
  void drawSettings() override;
  std::shared_ptr<Settings> settingsRef() override {
    return std::dynamic_pointer_cast<Settings>(settings);
  }
  std::shared_ptr<WebcamVideoSourceSettings> settings;
  std::vector<std::string> deviceNames;
private:
  ofVideoGrabber grabber;
  bool grabberReady = false;
  int lastDeviceId = -1;
};

#endif
