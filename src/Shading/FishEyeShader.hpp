//
//  FishEyeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FishEyeShader_hpp
#define FishEyeShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FishEyeSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Oscillator> amountOscillator;
  
  FishEyeSettings(std::string shaderId, json j, std::string name) :
  amount(std::make_shared<Parameter>("Amount", 0.5, 0.1, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, name) {
    parameters = { amount };
    oscillators = { amountOscillator };
    registerParameters();
    load(j);
  };
};

class FishEyeShader: public Shader {
public:
  
  FishEyeSettings *settings;
  FishEyeShader(FishEyeSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/FishEye");
    shader.load("shaders/FishEye");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("amount", fmin(settings->amount->value, 0.999));
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
    return ShaderTypeFishEye;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif
