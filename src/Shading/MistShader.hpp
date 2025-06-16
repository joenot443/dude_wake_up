//
//  MistShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MistShader_hpp
#define MistShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MistSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mainColor;
  std::shared_ptr<Parameter> secondaryColor;
  MistSettings(std::string shaderId, json j) :
  mainColor(std::make_shared<Parameter>("Main Color", ParameterType_Color)),
  secondaryColor(std::make_shared<Parameter>("Secondary Color", ParameterType_Color)),
  ShaderSettings(shaderId, j, "Mist") {
    mainColor->setColor({.9, .8, .6, 1.0});
    secondaryColor->setColor({.1, .1, .2});
    parameters = { mainColor, secondaryColor };
    load(j);
    registerParameters();
  };
};

struct MistShader: Shader {
  MistSettings *settings;
  MistShader(MistSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Mist");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("mainColor", settings->mainColor->color->data()[0], settings->mainColor->color->data()[1], settings->mainColor->color->data()[2]);
    shader.setUniform3f("secondaryColor", settings->secondaryColor->color->data()[0], settings->secondaryColor->color->data()[1], settings->secondaryColor->color->data()[2]);
    shader.setUniform1f("v_time", ofGetElapsedTimef());
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
    return ShaderTypeMist;
  }

  void drawSettings() override {
    CommonViews::ShaderColor(settings->mainColor);
    CommonViews::ShaderColor(settings->secondaryColor);
  }
};

#endif
