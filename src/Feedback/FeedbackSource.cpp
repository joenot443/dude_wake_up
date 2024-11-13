//
//  FeedbackSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/12/23.
//

#include <stdio.h>
#include "FeedbackSource.hpp"
#include "LayoutStateService.hpp"
#include "FeedbackSourceService.hpp"
#include "Shader.hpp"

bool FeedbackSource::beingConsumed() {
  return FeedbackSourceService::getService()->isSourceBeingConsumed(id);
}

void FeedbackSource::resizeIfNecessary() {
  if (frameBuffer[0]->getWidth() != LayoutStateService::getService()->resolution.x) {
    setup();
  }
}

void FeedbackSource::setup() {
  frameBuffer.clear();
  log("Resizing FeedbackSource");

  // Add FrameBufferCount ofFbo to frameBuffer
  for (int i = 0; i < FrameBufferCount; i++) {
    std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
    fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
    fbo->begin();
    ofSetColor(0, 0, 0, 0);
    ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    fbo->end();
    frameBuffer.push_back(fbo);
  }
  
  hasBeenPrimed = false;
}

void FeedbackSource::primeFrameBuffer(std::shared_ptr<ofFbo> fbo) {
  setup();
  
  for (int i = 0; i < FrameBufferCount; i++) {
    frameBuffer[i]->begin();
    fbo->draw(0, 0);
    frameBuffer[i]->end();
  }
  hasBeenPrimed = true;
}
