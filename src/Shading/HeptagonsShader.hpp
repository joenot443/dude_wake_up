//
//  HeptagonsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HeptagonsShader_hpp
#define HeptagonsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct HeptagonsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> colorThreshold;
  std::shared_ptr<WaveformOscillator> colorThresholdOscillator;

  HeptagonsSettings(std::string shaderId, json j) :
  color(std::make_shared<Parameter>("Color Tint", ParameterType_Color)),
  colorThreshold(std::make_shared<Parameter>("Color Threshold", 0.8, 0.0, 1.0)),
  colorThresholdOscillator(std::make_shared<WaveformOscillator>(colorThreshold)),
  ShaderSettings(shaderId, j, "Heptagons") {
    parameters = { color, colorThreshold };
    oscillators = { colorThresholdOscillator };
    load(j);
    registerParameters();
  };
};

struct HeptagonsShader: Shader {
  HeptagonsSettings *settings;
  HeptagonsShader(HeptagonsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Heptagons");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform1f("colorThreshold", settings->colorThreshold->value);
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
    return ShaderTypeHeptagons;
  }

  void drawSettings() override {
    CommonViews::ShaderColor(settings->color);
  }
};

#endif
