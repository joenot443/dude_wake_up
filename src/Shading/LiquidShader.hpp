//
//  LiquidShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LiquidShader_hpp
#define LiquidShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct LiquidSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> intensity;
  std::shared_ptr<WaveformOscillator> intensityOscillator;
  
  // New parameters for advanced liquid effects
  std::shared_ptr<Parameter> flowSpeed;
  std::shared_ptr<WaveformOscillator> flowSpeedOscillator;
  
  std::shared_ptr<Parameter> turbulence;
  std::shared_ptr<WaveformOscillator> turbulenceOscillator;

  LiquidSettings(std::string shaderId, json j, std::string name) :
    ShaderSettings(shaderId, j, name),
    intensity(std::make_shared<Parameter>("Intensity", 0.2, 0.0, 1.0)),
    flowSpeed(std::make_shared<Parameter>("Flow Speed", 0.8, 0.1, 2.0)),
    turbulence(std::make_shared<Parameter>("Turbulence", 1.0, 0.0, 3.0)),
    intensityOscillator(std::make_shared<WaveformOscillator>(intensity)),
    flowSpeedOscillator(std::make_shared<WaveformOscillator>(flowSpeed)),
    turbulenceOscillator(std::make_shared<WaveformOscillator>(turbulence))
  {
    parameters = {intensity, flowSpeed, turbulence};
    oscillators = {intensityOscillator, flowSpeedOscillator, turbulenceOscillator};
    load(j);
    registerParameters();
  }
};

class LiquidShader: public Shader {
public:
  LiquidSettings *settings;
  LiquidShader(LiquidSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Liquid");
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeLiquid;
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("intensity", settings->intensity->value);
    shader.setUniform1f("flowSpeed", settings->flowSpeed->value);
    shader.setUniform1f("turbulence", settings->turbulence->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->intensity, settings->intensityOscillator);
    CommonViews::ShaderParameter(settings->flowSpeed, settings->flowSpeedOscillator);
    CommonViews::ShaderParameter(settings->turbulence, settings->turbulenceOscillator);
  }
};

#endif
