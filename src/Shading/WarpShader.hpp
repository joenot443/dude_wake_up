//
//  WarpShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WarpShader_hpp
#define WarpShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WarpSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<WaveformOscillator> amountOscillator;
  
  WarpSettings(std::string shaderId, json j, std::string name) :
  amount(std::make_shared<Parameter>("amount", 1.0, 0.0, 2.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, name) {
    parameters = { amount };
    oscillators = { amountOscillator };
    load(j);
    registerParameters();
  };
};

class WarpShader: public Shader {
public:
  WarpSettings *settings;
  WarpShader(WarpSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Warp");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypeWarp;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Warp");
    
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif
