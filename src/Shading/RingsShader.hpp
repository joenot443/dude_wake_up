//
//  RingsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RingsShader_hpp
#define RingsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RingsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> ringSize;
  std::shared_ptr<WaveformOscillator> ringSizeOscillator;
  
  std::shared_ptr<Parameter> color;
  
  std::shared_ptr<Parameter> ringCount;
public:
  RingsSettings(std::string shaderId, json j, std::string name) :
  ringSize(std::make_shared<Parameter>("Ring Size", 5.0, 0.1, 10.0)),
  color(std::make_shared<Parameter>("Color", ParameterType_Color)),
  ringCount(std::make_shared<Parameter>("Ring Count", 10.0, 1.0, 100.0, ParameterType_Int)),
  ringSizeOscillator(std::make_shared<WaveformOscillator>(ringSize)),
  ShaderSettings(shaderId, j, name) {
    // Default to yellow
    color->setColor({1.0, 1.0, 0.0, 1.0});
    parameters = { ringSize, ringCount };
    oscillators = { ringSizeOscillator };
    registerParameters();
  };
};

class RingsShader: public Shader {
public:
  
  RingsSettings *settings;
  RingsShader(RingsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Rings");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("ringSize", settings->ringSize->value);
    shader.setUniform1i("ringCount", settings->ringCount->intValue);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 0;
  }
  
  ShaderType type() override {
    return ShaderTypeRings;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->ringSize, settings->ringSizeOscillator);
    CommonViews::ShaderIntParameter(settings->ringCount);
    CommonViews::ShaderColor(settings->color);
  }
};

#endif
