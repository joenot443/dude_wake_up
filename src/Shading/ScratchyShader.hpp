//
//  ScratchyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ScratchyShader_hpp
#define ScratchyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ScratchySettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount; // New parameter
  std::shared_ptr<Parameter> direction;
  std::shared_ptr<WaveformOscillator> amountOscillator; // New oscillator
  

  ScratchySettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("amount", 0.1, 0.0, 1.0)),
  direction(std::make_shared<Parameter>("direction", ParameterType_Bool)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)), // Initialize new oscillator
  ShaderSettings(shaderId, j, "Scratchy") {
    parameters = { amount, direction }; // Add amount to parameters
    oscillators = { amountOscillator }; // Add amountOscillator to oscillators
    load(j);
    registerParameters();
  };
};

struct ScratchyShader: Shader {
  ScratchySettings *settings;
  ScratchyShader(ScratchySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Scratchy");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value); // Set uniform for amount
    shader.setUniform1i("direction", settings->direction->boolValue); // Set uniform for direction
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
    return ShaderTypeScratchy;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator); // Add amount to drawSettings
    CommonViews::ShaderCheckbox(settings->direction);
  }
};

#endif
