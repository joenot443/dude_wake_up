//
//  DiffusionShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiffusionShader_hpp
#define DiffusionShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DiffusionSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  DiffusionSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "Diffusion") {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct DiffusionShader: Shader {
  DiffusionSettings *settings;
  std::shared_ptr<FeedbackSource> feedbackSource;

  DiffusionShader(DiffusionSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Diffusion");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
//    ofTexture fbTex = feedbackSource->getFrame(frameIndex());
//
//    canvas->begin();
//    shader.begin();
//    shader.setUniformTexture("tex", frame->getTexture(), 4);
//    shader.setUniform1f("shaderValue", settings->shaderValue->value);
//    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
//    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
//    frame->draw(0, 0);
//    shader.end();
//    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeDiffusion;
  }

  void drawSettings() override {
    CommonViews::H3Title("Diffusion");

  }
};

#endif /* DiffusionShader_hpp */
