//
//  RingsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RingsShader_hpp
#define RingsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct RingsSettings: public ShaderSettings {
  RingsSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct RingsShader: Shader {
  RingsSettings *settings;
  RingsShader(RingsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Rings");
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
    return ShaderTypeRings;
  }

  void drawSettings() override {
    CommonViews::H3Title("Rings");

  }
};

#endif /* RingsShader_hpp */