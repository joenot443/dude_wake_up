//
//  WebcamSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "WebcamSource.hpp"
#include "NodeLayoutView.hpp"
#include "CommonViews.hpp"

void WebcamSource::setup() {
  grabber.close();
  grabber.setDeviceID(settings->deviceId->intValue);
  grabber.setDesiredFrameRate(30);
  grabber.setPixelFormat(OF_PIXELS_RGBA);
  grabber.setup(settings->width->value, settings->height->value);
  fbo->allocate(settings->width->value, settings->height->value);
  maskShader.load("shaders/ColorKeyMaskMaker");

  // Collect the device names
  deviceNames.clear();
  std::vector<ofVideoDevice> devices = grabber.listDevices();
  for (int i = 0; i < devices.size(); i++)
  {
    deviceNames.push_back(devices[i].deviceName);
  }
}

void WebcamSource::saveFrame() {
  if (!grabber.isInitialized()) {
    return;
  }
  grabber.update();
  if (grabber.isFrameNew()) {
    if (settings->maskEnabled->boolValue == true)
    {
      fbo->begin();
      maskShader.begin();
      maskShader.setUniformTexture("tex", grabber.getTexture(), 0);
      maskShader.setUniform1f("time", ofGetElapsedTimef());
      maskShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
      maskShader.setUniform1i("drawTex", 1);
      maskShader.setUniform4f("chromaKey",
                              settings->maskColor->color->data()[0],
                              settings->maskColor->color->data()[1],
                              settings->maskColor->color->data()[2], 1.0);
      maskShader.setUniform1f("tolerance", settings->maskTolerance->value);
      maskShader.setUniform1i("invert", settings->invert->boolValue);
      ofClear(0, 0, 0, 255);
      ofClear(0, 0, 0, 0);

      grabber.draw(0, 0, fbo->getWidth(), fbo->getHeight());
      maskShader.end();
      fbo->end();
    }
    else
    {
      fbo->begin();
      ofClear(0, 0, 0, 255);
      grabber.draw(0, 0, fbo->getWidth(), fbo->getHeight());
      fbo->end();
    }
  }
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
  }
  return j;
}

void WebcamSource::drawSettings() {
  // Collect the names 
  if (CommonViews::ShaderOption(settings->deviceId, deviceNames)) {
    setup();
  }
  drawMaskSettings();
}
