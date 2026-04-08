//
//  WavyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WavyShader_hpp
#define WavyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WavySettings: public ShaderSettings {

  WavySettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "Wavy") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct WavyShader: Shader {
  WavySettings *settings;
  WavyShader(WavySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Wavy");
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
    return ShaderTypeWavy;
  }

  void drawSettings() override {
    CommonViews::H3Title("Wavy");

    
  }
};

#endif /* WavyShader_hpp */
