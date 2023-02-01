//
//  MountainsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MountainsShader_hpp
#define MountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct MountainsSettings: public ShaderSettings {
  MountainsSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct MountainsShader: Shader {
  MountainsSettings *settings;
  MountainsShader(MountainsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Mountains");
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
    return ShaderTypeMountains;
  }

  void drawSettings() override {
  }
};

#endif /* MountainsShader_hpp */
