//
//  SwirlShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SwirlShader_hpp
#define SwirlShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SwirlSettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  SwirlSettings(std::string shaderId, json j, std::string name) :
  amount(std::make_shared<Parameter>("Amount", 0.5, -3.0, 3.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "Swirl") {
    parameters = { amount };
    oscillators = { amountOscillator };
    load(j);
    registerParameters();
  };
};

struct SwirlShader: Shader {
  SwirlSettings *settings;
  SwirlShader(SwirlSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Swirl");
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
    return ShaderTypeSwirl;
  }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif
