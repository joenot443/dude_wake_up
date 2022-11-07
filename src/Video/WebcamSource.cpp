//
//  WebcamSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "WebcamSource.hpp"

void WebcamSource::setup() {
  grabber.setDeviceID(deviceID);
  grabber.setDesiredFrameRate(30);
  grabber.setPixelFormat(OF_PIXELS_RGB);
  grabber.setup(640, 480);
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(640, 480, GL_RGB);
  frameBuffer.bind(GL_SAMPLER_2D_RECT);
  frameBuffer.allocate(640*480*4, GL_STATIC_COPY);
}

void WebcamSource::update() {
  grabber.update();
  if (grabber.isFrameNew()) {
    grabber.getTexture().copyTo(frameBuffer);
    frameTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);
  }
}

