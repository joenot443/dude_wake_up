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

class WebcamSource : public VideoSource {
  
public:
  WebcamSource(std::string id, std::string name, int deviceID) : VideoSource(id, name, VideoSource_webcam), deviceID(deviceID) {};
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  

private:
  int deviceID;
  ofVideoGrabber grabber;
};

#endif
