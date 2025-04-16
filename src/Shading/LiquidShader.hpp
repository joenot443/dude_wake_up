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

  LiquidSettings(std::string shaderId, json j, std::string name) :
    ShaderSettings(shaderId, j, name),
    intensity(std::make_shared<Parameter>("Intensity", 1.0, 0.0, 2.0)), // Default, Min, Max
    intensityOscillator(std::make_shared<WaveformOscillator>(intensity))
  {
    parameters = {intensity};
    oscillators = {intensityOscillator};
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
    shader.setUniform1f("intensity", settings->intensity->value);  // Apply the intensity parameter
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->intensity, settings->intensityOscillator);
  }
};

#endif
