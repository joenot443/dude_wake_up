//
//  CompanionsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CompanionsShader_hpp
#define CompanionsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CompanionsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> positionX;
  std::shared_ptr<Parameter> positionY;
  std::shared_ptr<WaveformOscillator> positionXOscillator;
  std::shared_ptr<WaveformOscillator> positionYOscillator;

  CompanionsSettings(std::string shaderId, json j) :
  positionX(std::make_shared<Parameter>("positionX", 0.5, 0.0, 1.0)),
  positionY(std::make_shared<Parameter>("positionY", 0.5, 0.0, 1.0)),
  positionXOscillator(std::make_shared<WaveformOscillator>(positionX)),
  positionYOscillator(std::make_shared<WaveformOscillator>(positionY)),
  ShaderSettings(shaderId, j, "Companions") {
    parameters = { positionX, positionY };
    oscillators = { positionXOscillator, positionYOscillator };
    load(j);
    registerParameters();
  };
};

struct CompanionsShader: Shader {
  CompanionsSettings *settings;
  CompanionsShader(CompanionsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Companions");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("position", settings->positionX->value, settings->positionY->value);
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
    return ShaderTypeCompanions;
  }

  void drawSettings() override {
    CommonViews::H3Title("Companions");
    CommonViews::MultiSlider("Position", formatString("##position%s", shaderId.c_str()), settings->positionX, settings->positionY, settings->positionXOscillator, settings->positionYOscillator);
  }
};

#endif /* CompanionsShader_hpp */
