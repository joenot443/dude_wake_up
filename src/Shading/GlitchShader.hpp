//
//  GlitchShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/28/22.
//

#ifndef GlitchShader_hpp
#define GlitchShader_hpp
#include "ofMain.h"

#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct GlitchShader: Shader {
  ofShader shader;
public:
  void setup() override {
    shader.load("shaders/shader_glitch");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform1f("iTime", ofGetElapsedTimef());
    shader.setUniform3f("iResolution", canvas->getWidth(), canvas->getHeight(), 1.0);
    shader.setUniformTexture("iChannel0", frame->getTexture(), 6);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    ofClear(0,0,0,255);
  }
};

#endif /* GlitchShader_hpp */
