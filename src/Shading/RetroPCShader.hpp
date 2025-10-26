//
//  RetroPCShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RetroPCShader_hpp
#define RetroPCShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RetroPCSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> gamma;
  std::shared_ptr<WaveformOscillator> gammaOscillator;

  std::shared_ptr<Parameter> pixelSize;
  std::shared_ptr<WaveformOscillator> pixelSizeOscillator;

  std::shared_ptr<Parameter> ditherAmount;
  std::shared_ptr<WaveformOscillator> ditherAmountOscillator;

  RetroPCSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  gamma(std::make_shared<Parameter>("gamma", 1.0, 0.1, 3.0)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  pixelSize(std::make_shared<Parameter>("pixelSize", 4.0, 1.0, 16.0)),
  pixelSizeOscillator(std::make_shared<WaveformOscillator>(pixelSize)),
  ditherAmount(std::make_shared<Parameter>("ditherAmount", 1.0, 0.0, 1.0)),
  ditherAmountOscillator(std::make_shared<WaveformOscillator>(ditherAmount)),
  ShaderSettings(shaderId, j, "RetroPC") {
    parameters = { shaderValue, gamma, pixelSize, ditherAmount };
    oscillators = { shaderValueOscillator, gammaOscillator, pixelSizeOscillator, ditherAmountOscillator };
    load(j);
    registerParameters();
  };
};

struct RetroPCShader: Shader {
  RetroPCSettings *settings;
  RetroPCShader(RetroPCSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/RetroPC");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("gamma", settings->gamma->value);
    shader.setUniform1f("pixelSize", settings->pixelSize->value);
    shader.setUniform1f("ditherAmount", settings->ditherAmount->value);
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
    return ShaderTypeRetroPC;
  }

  void drawSettings() override {
    CommonViews::H3Title("RetroPC");

    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
    CommonViews::ShaderParameter(settings->pixelSize, settings->pixelSizeOscillator);
    CommonViews::ShaderParameter(settings->ditherAmount, settings->ditherAmountOscillator);
  }
};

#endif /* RetroPCShader_hpp */
