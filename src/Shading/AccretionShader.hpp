//
//  AccretionShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AccretionShader_hpp
#define AccretionShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AccretionSettings: public ShaderSettings {
  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AccretionSettings(std::string shaderId, json j) :
  brightness(std::make_shared<Parameter>("Brightness", 400.0, 50.0, 1000.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "Accretion") {
    parameters = { brightness };
    oscillators = { brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct AccretionShader: Shader {
  AccretionSettings *settings;
  AccretionShader(AccretionSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Accretion");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    ofClear(0, 0, 0, 255);
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
    return ShaderTypeAccretion;
  }

  void drawSettings() override {
    CommonViews::H3Title("Accretion");
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* AccretionShader_hpp */
