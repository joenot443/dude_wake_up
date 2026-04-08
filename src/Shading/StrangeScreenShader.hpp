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

  StrangeScreenSettings(std::string shaderId, json j) :
  cellCount(std::make_shared<Parameter>("Cell Count", 39.0, 5.0, 80.0, ParameterType_Int)),
  cellCountOscillator(std::make_shared<WaveformOscillator>(cellCount)),
  ShaderSettings(shaderId, j, "StrangeScreen") {
    parameters = { cellCount };
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
  }
};

#endif /* StrangeScreenShader_hpp */
