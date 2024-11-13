//
//  FeedbackSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 11/2/22.
//

#ifndef FeedbackSource_h
#define FeedbackSource_h
#include "ofMain.h"
#include "FramePreview.hpp"
#include "VideoSourceSettings.hpp"
#include "Connection.hpp"
#include "Console.hpp"

const int FrameBufferCount=30;

enum FeedbackType {
  FeedbackType_self, // Feedback frames are fed from the FeedbackSource itself. They are initially populated with the same 30 frames of the original source fbo
  FeedbackType_full, // Feedback frames are added from the source specified in FeedbackShader
};


struct FeedbackSource {
  FeedbackSource(std::string id, std::shared_ptr<VideoSourceSettings> sourceSettings) : id(id), sourceSettings(sourceSettings) {};

  std::shared_ptr<VideoSourceSettings> sourceSettings;
  std::string id;
  std::vector<std::shared_ptr<ofFbo>> frameBuffer = {};
  FeedbackType type = FeedbackType_full;
  bool hasBeenPrimed = false;
  
  int startIndex = 0;
  
  void setup();
  
  bool beingConsumed();
  
  void primeFrameBuffer(std::shared_ptr<ofFbo> fbo);
  
  void updateSettings(std::shared_ptr<VideoSourceSettings> settings) {
    sourceSettings = settings;
  }
  
  void teardown() {
    frameBuffer.clear();
  }
  
  void resizeIfNecessary();
  
  ofTexture mostRecentFrame() {
    return frameBuffer.at(startIndex)->getTexture();
  }
  
  ofTexture getFrame(int index) {
    if (!beingConsumed()) {
      log("Getting Feedback frame for a source not being consumed");
      ofLogFatalError();
    }
    
    int destIndex = (startIndex + index) % FrameBufferCount;
    auto fbo = frameBuffer.at(destIndex);
    return fbo->getTexture();
  }
  
  std::shared_ptr<ofFbo> getFbo(int index) {
    if (!beingConsumed()) {
      log("Getting Feedback frame for a source not being consumed");
      ofLogFatalError();
    }
    
    int destIndex = (startIndex + index) % FrameBufferCount;
    auto fbo = frameBuffer.at(destIndex);
    return fbo;
  }
  
  void clearFrameBuffer() {
    setup();
  }
  
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
    
    FramePreview::getInstance().setFrame(canvas);
    
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
