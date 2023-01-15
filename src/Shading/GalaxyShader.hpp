//
//  GalaxyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GalaxyShader_hpp
#define GalaxyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct GalaxySettings: public ShaderSettings {
  GalaxySettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct GalaxyShader: Shader {
  GalaxySettings *settings;
  GalaxyShader(GalaxySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("../../shaders/Galaxy");
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
    return ShaderTypeGalaxy;
  }

  void drawSettings() override {
    CommonViews::H3Title("Galaxy");

  }
};

#endif /* GalaxyShader_hpp */
