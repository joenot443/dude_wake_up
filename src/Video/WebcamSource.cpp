//
//  WebcamSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "WebcamSource.hpp"
#include "NodeLayoutView.hpp"
#include "CommonViews.hpp"
#include "LayoutStateService.hpp"

void WebcamSource::setup() {
  grabberReady = false;
  grabber.close();
  grabber.setDeviceID(settings->deviceId->intValue);
  grabber.setDesiredFrameRate(30);
  grabber.setPixelFormat(OF_PIXELS_RGBA);
  if (!grabber.setup(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y)) {
    return;
  }
  grabberReady = true;
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  maskShader.load("shaders/ColorKeyMaskMaker");

  // Collect the device names
  deviceNames.clear();
  std::vector<ofVideoDevice> devices = grabber.listDevices();

  for (int i = 0; i < devices.size(); i++)
  {
    deviceNames.push_back(devices[i].deviceName);
  }
  // Sort the deviceNames alphabetically
  std::sort(deviceNames.begin(), deviceNames.end());

  // Populate parameter options so the bridge exposes them to Swift
  settings->deviceId->options = deviceNames;
  lastDeviceId = settings->deviceId->intValue;
}

void WebcamSource::saveFrame() {
  // Re-initialize if device selection changed (e.g. from SwiftUI inspector)
  if (settings->deviceId->intValue != lastDeviceId) {
    lastDeviceId = settings->deviceId->intValue;
    setup();
  }

  if (!grabberReady) {
    return;
  }
  grabber.update();
  if (!grabber.isFrameNew()) return;
  
  fbo->begin();
  ofClear(0, 0, 0, 255);
  grabber.draw(0, 0, LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  fbo->end();
}

void WebcamSource::load(json j) {
  if (!j.is_object()) {
    log("Error hydrating WebcamSource from json");
    return;
  }
  
  id = j["id"];
  sourceName = j["sourceName"];
  settings->load(j["settings"]);
}

json WebcamSource::serialize() {
  json j;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_webcam;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  } else {
    j["x"] = origin.x;
    j["y"] = origin.y;
  }
  return j;
}

void WebcamSource::drawSettings() {
  // Collect the names 
  if (CommonViews::ShaderOption(settings->deviceId, deviceNames)) {
    setup();
  }
}
