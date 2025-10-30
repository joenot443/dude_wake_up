//
//  DoubleBlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DoubleBlurShader_hpp
#define DoubleBlurShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DoubleBlurSettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  DoubleBlurSettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("Amount", 5.0, 0.0, 100.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.0, 2.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "DoubleBlur") {
    parameters = { amount, brightness };
    oscillators = { amountOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct DoubleBlurShader: Shader {
  DoubleBlurSettings *settings;
  DoubleBlurShader(DoubleBlurSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/DoubleBlur");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    
    // Intermediate FBO for the horizontal pass
    ofFbo tempFbo;
    tempFbo.allocate(frame->getWidth(), frame->getHeight());
    
    // Horizontal blur pass
    tempFbo.begin();
    shader.begin();
    ofClear(0, 255);
    ofClear(0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1i("direction", 0); // Horizontal direction
    frame->draw(0, 0);
    shader.end();
    tempFbo.end();

    // Vertical blur pass
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", tempFbo.getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", tempFbo.getWidth(), tempFbo.getHeight());
    shader.setUniform1i("direction", 1); // Vertical direction
    tempFbo.draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeDoubleBlur;
  }
  
  void drawSettings() override {


    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
