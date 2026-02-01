//
//  ElectricEelsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ElectricEelsShader_hpp
#define ElectricEelsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ElectricEelsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> orbX;
  std::shared_ptr<WaveformOscillator> orbXOscillator;

  std::shared_ptr<Parameter> orbY;
  std::shared_ptr<WaveformOscillator> orbYOscillator;

  ElectricEelsSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  orbX(std::make_shared<Parameter>("Orb X", 0.0, -2.0, 2.0)),
  orbXOscillator(std::make_shared<WaveformOscillator>(orbX)),
  orbY(std::make_shared<Parameter>("Orb Y", 0.0, -2.0, 2.0)),
  orbYOscillator(std::make_shared<WaveformOscillator>(orbY)),
  ShaderSettings(shaderId, j, "ElectricEels") {
    parameters = { shaderValue, orbX, orbY };
    oscillators = { shaderValueOscillator, orbXOscillator, orbYOscillator };
    load(j);
    registerParameters();
  };
};

struct ElectricEelsShader: Shader {
  ElectricEelsSettings *settings;
  ElectricEelsShader(ElectricEelsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/ElectricEels");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("orbX", settings->orbX->value);
    shader.setUniform1f("orbY", settings->orbY->value);
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
    return ShaderTypeElectricEels;
  }

  void drawSettings() override {
    CommonViews::H3Title("ElectricEels");
    CommonViews::MultiSlider("Orb Position", settings->shaderId, settings->orbX, settings->orbY, settings->orbXOscillator, settings->orbYOscillator);
  }
};

#endif /* ElectricEelsShader_hpp */
