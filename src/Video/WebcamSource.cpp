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
  grabber.setup(settings->width->value, settings->height->value);
  fbo->allocate(settings->width->value, settings->height->value);
}

void WebcamSource::saveFrame() {
  if (!grabber.isInitialized()) {
    return;
  }
  grabber.update();
  if (grabber.isFrameNew()) {
    fbo->begin();
    grabber.draw(0, 0);
    fbo->end();
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
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}
