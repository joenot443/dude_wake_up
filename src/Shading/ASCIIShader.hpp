//
//  AsciiShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AsciiShader_hpp
#define AsciiShader_hpp

#include "ofMain.h"

#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

// Ascii

struct AsciiSettings: public ShaderSettings  {
  std::shared_ptr<Parameter> enabled;
  
  AsciiSettings(std::string shaderId, json j) :
  enabled(std::make_shared<Parameter>("enabled", shaderId, 0.0,  1.0, 0.0)),
  ShaderSettings(shaderId)
  {
    parameters = {enabled};
    oscillators = {};
    load(j);
  }
};


struct AsciiShader: Shader {
  ofShader shader;
  AsciiSettings *settings;
  AsciiShader(AsciiSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/ascii");
  }
  
  ShaderType type() override {
    return ShaderTypeAscii;
  }
  
  bool enabled() override {
    return true;
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
  }
};

#endif /* AsciiShader_hpp */
