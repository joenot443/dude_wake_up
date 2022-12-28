//
//  EmptyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef EmptyShader_hpp
#define EmptyShader_hpp

#include "ofMain.h"

#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct EmptySettings: public ShaderSettings {
  EmptySettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct EmptyShader: Shader {
  EmptySettings *settings;
  EmptyShader(EmptySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("../../shaders/empty");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
};

#endif /* EmptyShader_hpp */
