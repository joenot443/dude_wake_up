//
//  DiscoShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiscoShader_hpp
#define DiscoShader_hpp

#include "WaveformOscillator.hpp"
#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "TimeService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include <stdio.h>

struct DiscoSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> scaleX;
  std::shared_ptr<Parameter> scaleY;
  std::shared_ptr<Parameter> timeScale;
  std::shared_ptr<Parameter> border;

  std::shared_ptr<WaveformOscillator> scaleXOscillator;
  std::shared_ptr<WaveformOscillator> scaleYOscillator;
  std::shared_ptr<WaveformOscillator> timeScaleOscillator;
  std::shared_ptr<WaveformOscillator> borderOscillator;

  DiscoSettings(std::string shaderId, json j, std::string name) :
  scaleX(std::make_shared<Parameter>("Scale X", 0.5, 0.0, 2.0)),
  scaleY(std::make_shared<Parameter>("Scale Y", 0.5, 0.0, 2.0)),
  timeScale(std::make_shared<Parameter>("Time Scale", 0.1, 0.0, 1.0)),
  border(std::make_shared<Parameter>("Border", 9.5, 0.0, 20.0)),
  scaleXOscillator(std::make_shared<WaveformOscillator>(scaleX)),
  scaleYOscillator(std::make_shared<WaveformOscillator>(scaleY)),
  timeScaleOscillator(std::make_shared<WaveformOscillator>(timeScale)),
  borderOscillator(std::make_shared<WaveformOscillator>(border)),
  ShaderSettings(shaderId, j, name) {
    parameters = { scaleX, scaleY, timeScale, border };
    oscillators = { scaleXOscillator, scaleYOscillator, timeScaleOscillator, borderOscillator };
    load(j);
    registerParameters();
  };
};

class DiscoShader: public Shader {
public:
  
  DiscoSettings *settings;
  DiscoShader(DiscoSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Disco");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("scaleX", settings->scaleX->value);
    shader.setUniform1f("scaleY", settings->scaleY->value);
    shader.setUniform1f("timeScale", settings->timeScale->value);
    shader.setUniform1f("border", settings->border->value);
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
    return ShaderTypeDisco;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->scaleX, settings->scaleXOscillator);
    CommonViews::ShaderParameter(settings->scaleY, settings->scaleYOscillator);
    CommonViews::ShaderParameter(settings->timeScale, settings->timeScaleOscillator);
    CommonViews::ShaderParameter(settings->border, settings->borderOscillator);
  }
};

#endif
