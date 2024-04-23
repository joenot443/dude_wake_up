//
//  AlphaMixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AlphaMixShader_hpp
#define AlphaMixShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AlphaMixSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<ValueOscillator> shaderValueOscillator;

  AlphaMixSettings(std::string shaderId, json j, std::string name) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 1.0  , -1.0, 2.0)),
  shaderValueOscillator(std::make_shared<ValueOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct AlphaMixShader: Shader {
  AlphaMixSettings *settings;
  AlphaMixShader(AlphaMixSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/AlphaMix");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeAlphaMix;
  }

  void drawSettings() override {
    CommonViews::H3Title("AlphaMix");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif /* AlphaMixShader_hpp */
