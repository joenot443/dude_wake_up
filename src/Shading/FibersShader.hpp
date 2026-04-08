//
//  FibersShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FibersShader_hpp
#define FibersShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FibersSettings: public ShaderSettings {

  FibersSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "Fibers") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct FibersShader: Shader {
  FibersSettings *settings;
  FibersShader(FibersSettings *settings) : settings(settings), Shader(settings) {};
 
  void setup() override {
    shader.load("shaders/Fibers");
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
    return ShaderTypeFibers;
  }

  void drawSettings() override {
    

    
  }
};

#endif
