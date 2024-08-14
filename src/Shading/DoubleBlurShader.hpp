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
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  
  DoubleBlurSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "DoubleBlur") {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
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
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1i("direction", 0); // Horizontal direction
    frame->draw(0, 0);
    shader.end();
    tempFbo.end();
    
    // Vertical blur pass
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", tempFbo.getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    CommonViews::H3Title("DoubleBlur");
    
    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif /* DoubleBlurShader_hpp */
