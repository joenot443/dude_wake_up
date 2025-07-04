//
//  SwirlingSoulShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SwirlingSoulShader_hpp
#define SwirlingSoulShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SwirlingSoulSettings: public ShaderSettings {
  std::shared_ptr<Parameter> centerX;
  std::shared_ptr<WaveformOscillator> centerXOscillator;
  std::shared_ptr<Parameter> centerY;
  std::shared_ptr<WaveformOscillator> centerYOscillator;

  std::shared_ptr<Parameter> size;
  std::shared_ptr<WaveformOscillator> sizeOscillator;

  SwirlingSoulSettings(std::string shaderId, json j, std::string name) :
  centerX(std::make_shared<Parameter>("centerX", 0.0, -1.0, 1.0)),
  centerY(std::make_shared<Parameter>("centerY", 0.0, -1.0, 1.0)),
  size(std::make_shared<Parameter>("size", 2.0, 0.0, 4.0)),
  centerXOscillator(std::make_shared<WaveformOscillator>(centerX)),
  centerYOscillator(std::make_shared<WaveformOscillator>(centerY)),
  sizeOscillator(std::make_shared<WaveformOscillator>(size)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { centerX, centerY, size };
    oscillators = { centerXOscillator, centerYOscillator, sizeOscillator };
    load(j);
    registerParameters();
  };
};

struct SwirlingSoulShader: Shader {
  SwirlingSoulSettings *settings;
  SwirlingSoulShader(SwirlingSoulSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/SwirlingSoul");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("size", settings->size->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform2f("center", settings->centerX->value + 1.0, settings->centerY->value + 1.0);
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
    return ShaderTypeSwirlingSoul;
  }

  void drawSettings() override {
    CommonViews::MultiSlider("Position", settings->centerX->paramId, settings->centerX, settings->centerY, settings->centerXOscillator, settings->centerYOscillator);
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
  }
};

#endif
