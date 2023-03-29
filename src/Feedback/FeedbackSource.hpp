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
#include "Console.hpp"

const int FrameBufferCount=30;

struct FeedbackSource {
  FeedbackSource(std::string id, std::string name, std::shared_ptr<VideoSourceSettings> sourceSettings) : name(name), id(id), sourceSettings(sourceSettings) {};

  std::shared_ptr<VideoSourceSettings> sourceSettings;
  std::string name;
  std::string id;
  std::vector<ofFbo> frameBuffer = {};
//  std::shared_ptr<Shader> consumingShader;
  int startIndex = 0;
  
  void setup() {
    frameBuffer.clear();
    
    // Add FrameBufferCount ofFbo to frameBuffer
    for (int i = 0; i < FrameBufferCount; i++) {
      ofFbo fbo;
      fbo.allocate(sourceSettings->width->intValue, sourceSettings->height->intValue, GL_RGBA);
      fbo.begin();
      ofClear(0,0,0,255);
      fbo.end();
      frameBuffer.push_back(fbo);
    }
  }
  
  void teardown() {
    frameBuffer.clear();
  }
  
  void resizeIfNecessary() {
    if (frameBuffer[0].getWidth() != sourceSettings->width->intValue) {
      setup();
    }
  }
  
  ofTexture mostRecentFrame() {
    return frameBuffer.at(startIndex).getTexture();
  }
  
  ofTexture getFrame(int index) {
    int destIndex = (startIndex + index) % FrameBufferCount;
    auto fbo = frameBuffer.at(destIndex);
    return fbo.getTexture();
  }
  
  void clearFrameBuffer() {
    frameBuffer.clear();

    for (int i = 0; i < FrameBufferCount; i++) {
      auto canvas = ofFbo();
      canvas.begin();
      ofClear(0,0,0,255);
      canvas.end();
      frameBuffer.push_back(canvas);
    }
  }
  
  void pushFrame(ofFbo fbo) {
    resizeIfNecessary();
    
    auto canvas = frameBuffer[startIndex];
    canvas.begin();
    ofClear(0,0,0,255);
    fbo.draw(0, 0);
    canvas.end();
    
    startIndex++;
    
    if (startIndex == FrameBufferCount) {
      startIndex = 0;
    }
  }
  
  void pushFrame(std::shared_ptr<ofTexture> frame) {
    resizeIfNecessary();
    
    auto canvas = frameBuffer[startIndex];
    canvas.begin();
    ofClear(0,0,0,255);
    frame->draw(0, 0, frame->getWidth(), frame->getHeight());
    canvas.end();
    
    startIndex++;
    
    if (startIndex == FrameBufferCount) {
      startIndex = 0;
    }
  }
};

#endif /* FeedbackSource_h */
