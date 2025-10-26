//
//  CircleBlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CircleBlurShader_hpp
#define CircleBlurShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CircleBlurSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> blurRadius;
  std::shared_ptr<WaveformOscillator> blurRadiusOscillator;

  std::shared_ptr<Parameter> channelSeparation;
  std::shared_ptr<WaveformOscillator> channelSeparationOscillator;

  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<WaveformOscillator> thresholdOscillator;

  CircleBlurSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  blurRadius(std::make_shared<Parameter>("Blur Radius", 0.05, 0.0, 0.2)),
  blurRadiusOscillator(std::make_shared<WaveformOscillator>(blurRadius)),
  channelSeparation(std::make_shared<Parameter>("Channel Separation", 1.0, 0.0, 1.0)),
  channelSeparationOscillator(std::make_shared<WaveformOscillator>(channelSeparation)),
  threshold(std::make_shared<Parameter>("Threshold", 0.35, 0.0, 1.0)),
  thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),
  ShaderSettings(shaderId, j, "CircleBlur") {
    parameters = { shaderValue, blurRadius, channelSeparation, threshold };
    oscillators = { shaderValueOscillator, blurRadiusOscillator, channelSeparationOscillator, thresholdOscillator };
    load(j);
    registerParameters();
  };
};

struct CircleBlurShader: Shader {
  CircleBlurSettings *settings;
  CircleBlurShader(CircleBlurSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/CircleBlur");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("blurRadius", settings->blurRadius->value);
    shader.setUniform1f("channelSeparation", settings->channelSeparation->value);
    shader.setUniform1f("threshold", settings->threshold->value);
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
    return ShaderTypeCircleBlur;
  }

  void drawSettings() override {
    CommonViews::H3Title("CircleBlur");

    CommonViews::ShaderParameter(settings->blurRadius, settings->blurRadiusOscillator);
    CommonViews::ShaderParameter(settings->channelSeparation, settings->channelSeparationOscillator);
    CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
  }
};

#endif /* CircleBlurShader_hpp */
