//
//  NikoShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef NikoShader_hpp
#define NikoShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct NikoSettings: public ShaderSettings {
  NikoSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct NikoShader: Shader {
  NikoSettings *settings;
  NikoShader(NikoSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Niko");
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

  ShaderType type() override {
    return ShaderTypeNiko;
  }

  void drawSettings() override {
    CommonViews::H3Title("Niko");

  }
};

#endif /* NikoShader_hpp */
