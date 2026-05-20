//
//  StrangeScreenShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StrangeScreenShader_hpp
#define StrangeScreenShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StrangeScreenSettings: public ShaderSettings {
  std::shared_ptr<Parameter> cellCount;
  std::shared_ptr<WaveformOscillator> cellCountOscillator;
  std::shared_ptr<Parameter> backgroundColor;

  StrangeScreenSettings(std::string shaderId, json j) :
  cellCount(std::make_shared<Parameter>("Cell Count", 39.0, 5.0, 80.0, ParameterType_Int)),
  cellCountOscillator(std::make_shared<WaveformOscillator>(cellCount)),
  backgroundColor(std::make_shared<Parameter>("Background Color", ParameterType_Color)),
  ShaderSettings(shaderId, j, "StrangeScreen") {
    backgroundColor->setColor({0.5f, 0.5f, 0.5f, 1.0f});
    parameters = { cellCount, backgroundColor };
    oscillators = { cellCountOscillator };
    load(j);
    registerParameters();
  };
};

struct StrangeScreenShader: Shader {
  StrangeScreenSettings *settings;
  StrangeScreenShader(StrangeScreenSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/StrangeScreen");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("cellCount", settings->cellCount->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("backgroundColor",
                        settings->backgroundColor->color->data()[0],
                        settings->backgroundColor->color->data()[1],
                        settings->backgroundColor->color->data()[2],
                        settings->backgroundColor->color->data()[3]);
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
    return ShaderTypeStrangeScreen;
  }

  void drawSettings() override {
    CommonViews::H3Title("StrangeScreen");
    CommonViews::ShaderIntParameter(settings->cellCount);
    CommonViews::ShaderColor(settings->backgroundColor);
  }
};

#endif /* StrangeScreenShader_hpp */
