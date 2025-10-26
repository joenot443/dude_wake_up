//
//  StrawberryWineShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StrawberryWineShader_hpp
#define StrawberryWineShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StrawberryWineSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> colorDesaturation;
  std::shared_ptr<WaveformOscillator> colorDesaturationOscillator;

  std::shared_ptr<Parameter> channelMix;
  std::shared_ptr<WaveformOscillator> channelMixOscillator;

  std::shared_ptr<Parameter> redIntensity;
  std::shared_ptr<WaveformOscillator> redIntensityOscillator;

  StrawberryWineSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  colorDesaturation(std::make_shared<Parameter>("Color Desaturation", 0.5, 0.0, 1.0)),
  colorDesaturationOscillator(std::make_shared<WaveformOscillator>(colorDesaturation)),
  channelMix(std::make_shared<Parameter>("Channel Mix", 0.5, 0.0, 1.0)),
  channelMixOscillator(std::make_shared<WaveformOscillator>(channelMix)),
  redIntensity(std::make_shared<Parameter>("Red Intensity", 1.0, 0.0, 2.0)),
  redIntensityOscillator(std::make_shared<WaveformOscillator>(redIntensity)),
  ShaderSettings(shaderId, j, "StrawberryWine") {
    parameters = { shaderValue, colorDesaturation, channelMix, redIntensity };
    oscillators = { shaderValueOscillator, colorDesaturationOscillator, channelMixOscillator, redIntensityOscillator };
    load(j);
    registerParameters();
  };
};

struct StrawberryWineShader: Shader {
  StrawberryWineSettings *settings;
  StrawberryWineShader(StrawberryWineSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/StrawberryWine");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("colorDesaturation", settings->colorDesaturation->value);
    shader.setUniform1f("channelMix", settings->channelMix->value);
    shader.setUniform1f("redIntensity", settings->redIntensity->value);
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
    return ShaderTypeStrawberryWine;
  }

  void drawSettings() override {
    CommonViews::H3Title("StrawberryWine");

    CommonViews::ShaderParameter(settings->colorDesaturation, settings->colorDesaturationOscillator);
    CommonViews::ShaderParameter(settings->channelMix, settings->channelMixOscillator);
    CommonViews::ShaderParameter(settings->redIntensity, settings->redIntensityOscillator);
  }
};

#endif /* StrawberryWineShader_hpp */
