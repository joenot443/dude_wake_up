//
//  LavaShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LavaShader_hpp
#define LavaShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct LavaSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> smoothness;
  std::shared_ptr<WaveformOscillator> smoothnessOscillator;

  std::shared_ptr<Parameter> blobCount;
  std::shared_ptr<WaveformOscillator> blobCountOscillator;

  LavaSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  smoothness(std::make_shared<Parameter>("Smoothness", 0.4, 0.1, 1.0)),
  smoothnessOscillator(std::make_shared<WaveformOscillator>(smoothness)),
  blobCount(std::make_shared<Parameter>("Blob Count", 8.0, 2.0, 16.0, ParameterType_Int)),
  blobCountOscillator(std::make_shared<WaveformOscillator>(blobCount)),
  ShaderSettings(shaderId, j, "Lava") {
    parameters = { shaderValue, smoothness, blobCount };
    oscillators = { shaderValueOscillator, smoothnessOscillator, blobCountOscillator };
    load(j);
    registerParameters();
  };
};

struct LavaShader: Shader {
  LavaSettings *settings;
  LavaShader(LavaSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Lava");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("smoothness", settings->smoothness->value);
    shader.setUniform1f("blobCount", settings->blobCount->value);
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
    return ShaderTypeLava;
  }

  void drawSettings() override {
    CommonViews::H3Title("Lava");
    CommonViews::ShaderParameter(settings->smoothness, settings->smoothnessOscillator);
    CommonViews::ShaderIntParameter(settings->blobCount);
  }
};

#endif /* LavaShader_hpp */
