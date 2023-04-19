//
//  WebcamSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "WebcamSource.hpp"
#include "NodeLayoutView.hpp"

void WebcamSource::setup() {
  auto devices = grabber.listDevices();
  
  grabber.setDeviceID(deviceID);
  grabber.setDesiredFrameRate(30);
  grabber.setPixelFormat(OF_PIXELS_RGBA);
  grabber.setup(settings.width->value, settings.height->value);
  
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(settings.width->value, settings.height->value, GL_RGBA);
}

void WebcamSource::saveFrame() {
  // If our width or height has changed, setup again
//  if (grabber.getWidth() != settings.width->value ||
//      grabber.getHeight() != settings.height->value) {
//    setup();
//  }
  if (!grabber.isInitialized()) {
    return;
  }
  grabber.update();
  if (grabber.isFrameNew()) {
    frameTexture = std::make_shared<ofTexture>(grabber.getTexture());
  }
}

void WebcamSource::load(json j) {
  if (!j.is_object()) {
    log("Error hydrating WebcamSource from json");
    return;
  }
  
  deviceID = j["deviceId"];
  id = j["id"];
  sourceName = j["sourceName"];
}

json WebcamSource::serialize() {
  json j;
  j["deviceId"] = deviceID;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_webcam;
  j["x"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.x;
  j["y"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.y;
  return j;
}
