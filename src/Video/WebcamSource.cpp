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
  
  previewTexture = std::make_shared<ofTexture>();
  previewTexture->allocate(320, 240, GL_RGB);
  previewBuffer.bind(GL_SAMPLER_2D_RECT);
  previewBuffer.allocate(320*240*4, GL_STATIC_COPY);
}

void WebcamSource::saveFrame() {
  grabber.update();
  if (grabber.isFrameNew()) {
    grabber.getTexture().copyTo(frameBuffer);
    grabber.getTexture().copyTo(previewBuffer);
    
    // Copy the grabber's texture to the frame texture
    frameTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);
    previewTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);
  }
}
