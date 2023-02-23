//
//  FeedbackSource.h
//  dude_wake_up
//
//  Created by Joe Crozier on 11/2/22.
//

#ifndef FeedbackSource_h
#define FeedbackSource_h
#include "ofMain.h"
#include "Console.hpp"

const int FrameBufferCount=30;

struct FeedbackSource {
  std::string name;
  std::string id;
  std::vector<ofFbo> frameBuffer = {};
//  std::shared_ptr<Shader> consumingShader;
  int startIndex = 0;
  
  void setup() {
    // Add FrameBufferCount ofFbo to frameBuffer

    for (int i = 0; i < FrameBufferCount; i++) {
      ofFbo fbo;
      fbo.allocate(1920, 1080, GL_RGBA);
      fbo.begin();
      ofClear(0,0,0,255);
      fbo.end();
      frameBuffer.push_back(fbo);
    }
  }

  FeedbackSource(std::string id, std::string name) : name(name), id(id) {};
  
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
  
  void pushFrame(std::shared_ptr<ofTexture> frame) {
//    if (consumingShader == nullptr) return;
    
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
