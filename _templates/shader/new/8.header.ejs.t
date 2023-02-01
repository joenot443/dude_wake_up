---
to: src/Shading/<%= name %>Shader.hpp
---

//
//  <%= name %>Shader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef <%= name %>Shader_hpp
#define <%= name %>Shader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct <%= name %>Settings: public ShaderSettings {
  <%= name %>Settings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct <%= name %>Shader: Shader {
  <%= name %>Settings *settings;
  <%= name %>Shader(<%= name %>Settings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/<%= name %>");
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
    return ShaderType<%= name %>;
  }

  void drawSettings() override {
    CommonViews::H3Title("<%= name %>");

  }
};

#endif /* <%= name %>Shader_hpp */
