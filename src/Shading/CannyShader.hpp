//
//  CannyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CannyShader_hpp
#define CannyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CannySettings: public ShaderSettings {
  std::shared_ptr<Parameter> backgroundColor;
  std::shared_ptr<Parameter> edgeColor;
  std::shared_ptr<Parameter> minThresh;
  std::shared_ptr<Parameter> maxThresh;
  
  CannySettings(std::string shaderId, json j, std::string name) :
  backgroundColor(std::make_shared<Parameter>("Background Color", 1.0, -1.0, 2.0, ParameterType_Color)),
  edgeColor(std::make_shared<Parameter>("Edge Color", 1.0, -1.0, 2.0, ParameterType_Color)),
  minThresh(std::make_shared<Parameter>("Min Threshold", 0.1, 0.0, 5.0)),
  maxThresh(std::make_shared<Parameter>("Max Threshold", 0.6, 0.0, 5.0)),
  ShaderSettings(shaderId, j, name) {
    edgeColor->setColor({1.0, 1.0, 1.0, 1.0});
    parameters = { backgroundColor, edgeColor };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct CannyShader: Shader {
  CannySettings *settings;
  CannyShader(CannySettings *settings) : settings(settings), Shader(settings) {};

  
  void setup() override {
    shader.load("shaders/Canny");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform4f("backgroundColor", settings->backgroundColor->color->data()[0], settings->backgroundColor->color->data()[1], settings->backgroundColor->color->data()[2], settings->backgroundColor->color->data()[3]);
    shader.setUniform4f("edgeColor", settings->edgeColor->color->data()[0], settings->edgeColor->color->data()[1], settings->edgeColor->color->data()[2], settings->edgeColor->color->data()[3]);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("minThresh", settings->minThresh->value);
    shader.setUniform1f("maxThresh", settings->maxThresh->value);
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
    return ShaderTypeCanny;
  }
  
  void drawSettings() override {
    
    
    CommonViews::ShaderColor(settings->backgroundColor);
    CommonViews::ShaderColor(settings->edgeColor);
    
//    CommonViews::ShaderParameter(settings->alpha, nullptr);
    CommonViews::ShaderParameter(settings->minThresh, nullptr);
    CommonViews::ShaderParameter(settings->maxThresh, nullptr);
  }
};

#endif
