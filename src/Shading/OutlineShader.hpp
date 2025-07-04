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
  std::shared_ptr<Parameter> weight;
  std::shared_ptr<WaveformOscillator> weightOscillator;

  std::shared_ptr<Parameter> fineness;
  std::shared_ptr<WaveformOscillator> finenessOscillator;

  std::shared_ptr<Parameter> color;

  OutlineSettings(std::string shaderId, json j) :
  weight(std::make_shared<Parameter>("Weight", 10.0, 0.0, 100.0)),
  color(std::make_shared<Parameter>("Color", ParameterType_Color)),
  fineness(std::make_shared<Parameter>("Fineness", 64.0, 1.0, 300.0)),
  weightOscillator(std::make_shared<WaveformOscillator>(weight)),
  finenessOscillator(std::make_shared<WaveformOscillator>(fineness)),
  ShaderSettings(shaderId, j, "Outline") {
    parameters = { weight, color, fineness };
    oscillators = { weightOscillator, finenessOscillator };
    load(j);
    registerParameters();
  };
};

struct OutlineShader: Shader {
  OutlineSettings *settings;
  OutlineShader(OutlineSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Stroke");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("weight", settings->weight->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("fineness", settings->fineness->value);
    shader.setUniform4f("strokeColor", 
      settings->color->color->data()[0],
      settings->color->color->data()[1],
      settings->color->color->data()[2],
      1.0); // Full opacity for the stroke
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
    CommonViews::ShaderParameter(settings->weight, settings->weightOscillator);
    CommonViews::ShaderParameter(settings->fineness, settings->finenessOscillator);
    CommonViews::ShaderColor(settings->color);
  }
};

#endif
