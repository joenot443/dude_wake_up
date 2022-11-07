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
  std::vector<ofTexture> frameBuffer = {};

  FeedbackSource(std::string name, std::string id) : name(name), id(id) {};
  
  ofTexture getFrame(int index) {
    if (index < frameBuffer.size()) {
      return frameBuffer[index];
    } else {
      log("Missing feedback texture");
      ofTexture texture;
      texture.allocate(0, 0, GL_LUMINANCE_ALPHA);
      return texture;
    }
  }
  
  void clearFrameBuffer() {
    frameBuffer.clear();
    
    for (int i = 0; i < FrameBufferCount; i++) {
      ofFbo frame = ofFbo();
      frame.allocate(ofGetWidth(), ofGetHeight());
      frame.begin();
      ofClear(0,0,0,255);
      frame.end();
      frameBuffer.push_back(frame.getTexture());
    }
  }
  
  void pushFrame(ofTexture frame) {
    static ofFbo canvas;
    float width = frame.getWidth();
    float height = frame.getHeight();
    canvas.allocate(width, height);
    canvas.begin();
    ofClear(0,0,0,255);
    frame.draw(0, 0, width, height);
    canvas.end();
    auto ret = canvas.getTexture();
    frameBuffer.insert(frameBuffer.begin(), ret);
    
    if (frameBuffer.size() >= FrameBufferCount) {
      frameBuffer.pop_back();
    }
  }
  
};

#endif /* FeedbackSource_h */
