//
//  FeedbackSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 11/2/22.
//

#ifndef FeedbackSource_h
#define FeedbackSource_h
#include "ofMain.h"
#include "VideoSourceSettings.hpp"
#include "Connection.hpp"
#include "Console.hpp"

const int FrameBufferCount=30;

struct FeedbackSource {
  FeedbackSource(std::string id, std::shared_ptr<VideoSourceSettings> sourceSettings) : id(id), sourceSettings(sourceSettings) {};

  std::shared_ptr<VideoSourceSettings> sourceSettings;
  std::string id;
  std::vector<std::shared_ptr<ofFbo>> frameBuffer = {};

  int startIndex = 0;
  
  void setup() {
    frameBuffer.clear();
    log("Resizing FeedbackSource");

    // Add FrameBufferCount ofFbo to frameBuffer
    for (int i = 0; i < FrameBufferCount; i++) {
      std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
      fbo->allocate(sourceSettings->width->intValue, sourceSettings->height->intValue, GL_RGBA);
      fbo->begin();
      ofSetColor(0, 0, 0, 0);
      ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
      ofClear(0,0,0, 255);
      ofClear(0,0,0, 0);
      fbo->end();
      frameBuffer.push_back(fbo);
    }
  }
  
  bool beingConsumed();
  
  void updateSettings(std::shared_ptr<VideoSourceSettings> settings) {
    sourceSettings = settings;
  }
  
  void teardown() {
    frameBuffer.clear();
  }
  
  void resizeIfNecessary() {
    if (frameBuffer[0]->getWidth() != sourceSettings->width->intValue) {
      setup();
    }
  }
  
  ofTexture mostRecentFrame() {
    return frameBuffer.at(startIndex)->getTexture();
  }
  
  ofTexture getFrame(int index) {
    if (!beingConsumed()) {
      log("Getting Feedback frame for a source not being consumed");
    }
    
    int destIndex = (startIndex + index) % FrameBufferCount;
    auto fbo = frameBuffer.at(destIndex);
    return fbo->getTexture();
  }
  
  void clearFrameBuffer() {
    setup();
  }
  
  void shadeFrame(std::shared_ptr<Connectable> shader);
  
  void pushFrame(std::shared_ptr<ofFbo> fbo) {
    // Return if we have no consumers
    if (!beingConsumed()) return;
    
    resizeIfNecessary();
    
    auto canvas = frameBuffer[startIndex];
    canvas->begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    fbo->draw(0, 0);
    canvas->end();
    
    startIndex++;
    
    if (startIndex == FrameBufferCount) {
      startIndex = 0;
    }
  }
  
  void pushFrame(std::shared_ptr<ofTexture> frame) {
    // Return if we have no consumers
    if (!beingConsumed()) return;
    resizeIfNecessary();
    auto canvas = frameBuffer[startIndex];
    canvas->begin();
    frame->draw(0, 0, frame->getWidth(), frame->getHeight());
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    canvas->end();
    
    startIndex++;
    
    if (startIndex == FrameBufferCount) {
      startIndex = 0;
    }
  }
};

#endif /* FeedbackSource_h */
