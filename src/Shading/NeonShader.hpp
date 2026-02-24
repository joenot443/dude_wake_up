//
//  NeonShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef NeonShader_hpp
#define NeonShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct NeonSettings: public ShaderSettings {
  std::shared_ptr<Parameter> intensity;
  std::shared_ptr<WaveformOscillator> intensityOscillator;

  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> glow;
  std::shared_ptr<WaveformOscillator> glowOscillator;

  NeonSettings(std::string shaderId, json j) :
  intensity(std::make_shared<Parameter>("Intensity", 1.5, 0.0, 5.0)),
  intensityOscillator(std::make_shared<WaveformOscillator>(intensity)),
  speed(std::make_shared<Parameter>("Speed", 0.4, 0.0, 2.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  glow(std::make_shared<Parameter>("Glow", 0.005, 0.001, 0.05)),
  glowOscillator(std::make_shared<WaveformOscillator>(glow)),
  ShaderSettings(shaderId, j, "Neon") {
    parameters = { intensity, speed, glow };
    oscillators = { intensityOscillator, speedOscillator, glowOscillator };
    load(j);
    registerParameters();
  };
};

struct NeonShader: Shader {
  NeonSettings *settings;
  NeonShader(NeonSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Neon");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("intensity", settings->intensity->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("glow", settings->glow->value);
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
    return ShaderTypeNeon;
  }

  void drawSettings() override {
    CommonViews::H3Title("Neon");
    CommonViews::ShaderParameter(settings->intensity, settings->intensityOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->glow, settings->glowOscillator);
  }
};

#endif /* NeonShader_hpp */
