//
//  WelcomeRingsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WelcomeRingsShader_hpp
#define WelcomeRingsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WelcomeRingsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> cycles;
  std::shared_ptr<WaveformOscillator> cyclesOscillator;
  
  std::shared_ptr<Parameter> palette;

  WelcomeRingsSettings(std::string shaderId, json j) :
  cycles(std::make_shared<Parameter>("Cycles", 4.0, 1.0, 8.0)),
  cyclesOscillator(std::make_shared<WaveformOscillator>(cycles)),
  palette(std::make_shared<Parameter>("Palette", 0.0, 0.0, 7.0, ParameterType_Int)),
  ShaderSettings(shaderId, j, "WelcomeRings") {
    parameters = { cycles };
    oscillators = { cyclesOscillator };
    audioReactiveParameter = cycles;
    load(j);
    registerParameters();
  };
};

struct WelcomeRingsShader: Shader {
  WelcomeRingsSettings *settings;
  WelcomeRingsShader(WelcomeRingsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/WelcomeRings");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("cycles", settings->cycles->value);
    shader.setUniform1i("palette", settings->palette->intValue);
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
    return ShaderTypeWelcomeRings;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->cycles, settings->cyclesOscillator);
    CommonViews::ShaderIntParameter(settings->palette);
  }
};

#endif
