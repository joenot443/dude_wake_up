//
//  WobbleShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WobbleShader_hpp
#define WobbleShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <string>
#include <stdio.h>

struct WobbleSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> amount;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> amountOscillator;

  WobbleSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", shaderId, 1.0, 0.0, 2.0)),
  amount(std::make_shared<Parameter>("amount", shaderId, 1.0, 0.0, 2.0)),
  speedOscillator(std::make_shared<Oscillator>(speed)),
  amountOscillator(std::make_shared<Oscillator>(amount)),
  ShaderSettings(shaderId) {
    
  };
};

struct WobbleShader: Shader {
  WobbleSettings *settings;
  WobbleShader(WobbleSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Wobble");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("amount", settings->amount->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeWobble;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Wobble");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* WobbleShader_hpp */