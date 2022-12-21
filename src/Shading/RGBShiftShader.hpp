//
//  RGBShiftShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RGBShiftShader_hpp
#define RGBShiftShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct RGBShiftSettings: public ShaderSettings {
  RGBShiftSettings(std::string shaderId, json j) :  
  ShaderSettings(shaderId) {
    
  };
};

struct RGBShiftShader: Shader {
  RGBShiftSettings *settings;
  RGBShiftShader(RGBShiftSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/RGBShift");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
  
  ShaderType type() override {
    return ShaderTypeRGBShift;
  }

  void drawSettings() override {
    CommonViews::H3Title("RGBShift");

  }
};

#endif /* RGBShiftShader_hpp */
