//
//  GodRayShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GodRayShader_hpp
#define GodRayShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GodRaySettings: public ShaderSettings {

  GodRaySettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "GodRay") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct GodRayShader: Shader {
  GodRaySettings *settings;
  GodRayShader(GodRaySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/GodRay");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeGodRay;
  }

  void drawSettings() override {
    CommonViews::H3Title("GodRay");

    
  }
};

#endif /* GodRayShader_hpp */
