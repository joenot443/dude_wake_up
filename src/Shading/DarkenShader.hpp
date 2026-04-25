//
//  DarkenShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DarkenShader_hpp
#define DarkenShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DarkenSettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  DarkenSettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("Amount", 0.0, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "Darken") {
    parameters = { amount };
    oscillators = { amountOscillator };
    load(j);
    registerParameters();
  };
};

struct DarkenShader: Shader {
  DarkenSettings *settings;
  DarkenShader(DarkenSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Darken");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
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
    return ShaderTypeDarken;
  }

  void drawSettings() override {
    CommonViews::H3Title("Darken");

    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* DarkenShader_hpp */
