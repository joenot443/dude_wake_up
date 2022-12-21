//
//  CorbinShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CorbinShader_hpp
#define CorbinShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct CorbinSettings: public ShaderSettings {
  CorbinSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct CorbinShader: Shader {
  CorbinSettings *settings;
  CorbinShader(CorbinSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Corbin");
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

  void drawSettings() override {
    CommonViews::H3Title("Corbin");

  }
};

#endif /* CorbinShader_hpp */
