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

  const int rx = LayoutStateService::getService()->resolution.x;
  const int ry = LayoutStateService::getService()->resolution.y;

  // Pixel-format negotiation. Different backends expose different formats:
  //   - macOS AVFoundation: usually delivers RGBA cleanly
  //   - Windows Media Foundation / DirectShow: RGB is far more likely to
  //     succeed; RGBA often requires a shim conversion that some backends
  //     silently skip, leaving the texture uninitialized
  // Try RGB first, then RGBA. Deliberately do NOT fall through to
  // OF_PIXELS_NATIVE — NATIVE hands us MJPEG/YUY2 raw bytes that
  // ofVideoGrabber can't upload as a GL_RGB texture, producing garbage.
  static const ofPixelFormat kFormats[] = { OF_PIXELS_RGB, OF_PIXELS_RGBA };
  const char* kFormatNames[] = { "RGB", "RGBA" };

  for (size_t i = 0; i < sizeof(kFormats)/sizeof(kFormats[0]); ++i) {
    grabber.setPixelFormat(kFormats[i]);
    if (grabber.setup(rx, ry)) {
      ofLogNotice("WebcamSource") << "device " << settings->deviceId->intValue
        << " negotiated " << kFormatNames[i] << " @ " << rx << "x" << ry;
      grabberReady = true;
      break;
    }
    ofLogWarning("WebcamSource") << "device " << settings->deviceId->intValue
      << " rejected " << kFormatNames[i] << ", trying next";
    grabber.close();
  }

  if (!grabberReady) {
    ofLogError("WebcamSource") << "device " << settings->deviceId->intValue
      << " accepted neither RGB nor RGBA @ " << rx << "x" << ry
      << " — leaving source inactive";
    return;
  }

  fbo->allocate(rx, ry);
  optionalFbo->allocate(rx, ry);
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
