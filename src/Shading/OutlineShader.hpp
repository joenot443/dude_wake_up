//
//  OutlineShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OutlineShader_hpp
#define OutlineShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OutlineSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  OutlineSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "Outline") {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct OutlineShader: Shader {
  OutlineSettings *settings;
  OutlineShader(OutlineSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Outline");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypeOutline;
  }

  void drawSettings() override {
    CommonViews::H3Title("Outline");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif /* OutlineShader_hpp */