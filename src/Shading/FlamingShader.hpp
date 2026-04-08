//
//  FlamingShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FlamingShader_hpp
#define FlamingShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FlamingSettings: public ShaderSettings {
  std::shared_ptr<Parameter> zOffset;
  std::shared_ptr<WaveformOscillator> zOffsetOscillator;

  std::shared_ptr<Parameter> turbulence;
  std::shared_ptr<WaveformOscillator> turbulenceOscillator;

  FlamingSettings(std::string shaderId, json j) :
  zOffset(std::make_shared<Parameter>("Z Offset", 4.0, 1.0, 10.0)),
  zOffsetOscillator(std::make_shared<WaveformOscillator>(zOffset)),
  turbulence(std::make_shared<Parameter>("Turbulence", 0.4, 0.1, 1.0)),
  turbulenceOscillator(std::make_shared<WaveformOscillator>(turbulence)),
  ShaderSettings(shaderId, j, "Flaming") {
    parameters = { zOffset, turbulence };
    oscillators = { zOffsetOscillator, turbulenceOscillator };
    load(j);
    registerParameters();
  };
};

struct FlamingShader: Shader {
  FlamingSettings *settings;
  FlamingShader(FlamingSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Flaming");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("zOffset", settings->zOffset->value);
    shader.setUniform1f("turbulence", settings->turbulence->value);
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
    return ShaderTypeFlaming;
  }

  void drawSettings() override {
    CommonViews::H3Title("Flaming");
    CommonViews::ShaderParameter(settings->zOffset, settings->zOffsetOscillator);
    CommonViews::ShaderParameter(settings->turbulence, settings->turbulenceOscillator);
  }
};

#endif /* FlamingShader_hpp */
