//
//  RainbowRotatorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RainbowRotatorShader_hpp
#define RainbowRotatorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RainbowRotatorSettings: public ShaderSettings {
  RainbowRotatorSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

struct RainbowRotatorShader: Shader {
  RainbowRotatorSettings *settings;
  RainbowRotatorShader(RainbowRotatorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/RainbowRotator");
#endif
#ifdef RELEASE
shader.load("shaders/RainbowRotator");
#endif
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeRainbowRotator;
  }

  void drawSettings() override {
  }
};

#endif /* RainbowRotatorShader_hpp */
