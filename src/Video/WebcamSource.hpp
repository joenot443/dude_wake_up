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
  : VideoSourceSettings(sourceId, j), deviceId(std::make_shared<Parameter>("Device", 0.0, 0.0, 100.0)) {
    parameters.push_back(deviceId);
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
  std::shared_ptr<WebcamVideoSourceSettings> settings;
  std::vector<std::string> deviceNames;
private:
  ofVideoGrabber grabber;
};

#endif
