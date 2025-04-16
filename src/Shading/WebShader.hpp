//
//  WebShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WebShader_hpp
#define WebShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WebSettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  WebSettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("amount", 1.0, 0.0, 3.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "Web") {
    parameters = { amount };
    oscillators = { amountOscillator };
    audioReactiveParameter = amount;
    load(j);
    registerParameters();
  };
};

struct WebShader: Shader {
  WebSettings *settings;
  WebShader(WebSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Web");
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
    return ShaderTypeWeb;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* WebShader_hpp */
