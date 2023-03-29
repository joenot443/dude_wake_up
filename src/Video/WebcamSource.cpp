//
//  WebcamSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "WebcamSource.hpp"

void WebcamSource::setup() {
  auto devices = grabber.listDevices();
  
  grabber.setDeviceID(deviceID);
  grabber.setDesiredFrameRate(30);
  grabber.setPixelFormat(OF_PIXELS_RGB);
  grabber.setup(640, 480);
  
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(640, 480, GL_RGB);
  frameBuffer.bind(GL_SAMPLER_2D_RECT);
  frameBuffer.allocate(640*480*4, GL_STATIC_COPY);
}

void WebcamSource::saveFrame() {
  grabber.update();
  if (grabber.isFrameNew()) {
    grabber.getTexture().copyTo(frameBuffer);
    
    // Copy the grabber's texture to the frame texture
    frameTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);
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
  return j;
}
