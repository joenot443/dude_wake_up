//
//  PixelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelShader_hpp
#define PixelShader_hpp

#include "ofMain.h"
#include "VideoSettings.h"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct PixelShader: Shader {
  PixelSettings *settings;
  PixelShader(PixelSettings *settings) : settings(settings) {};
  
  ofShader shader;
  void setup() override {
    shader.load("shadersGL2/new/pixel");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("size", settings->size.value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
  
  bool enabled() override {
    return settings->enabled.boolValue;
  }
};

#endif /* PixelShader_hpp */
