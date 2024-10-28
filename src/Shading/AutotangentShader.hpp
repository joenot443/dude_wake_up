//
//  AutotangentShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AutotangentShader_hpp
#define AutotangentShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AutotangentSettings: public ShaderSettings {
  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AutotangentSettings(std::string shaderId, json j) :
  brightness(std::make_shared<Parameter>("brightness", 0.2, 0.0, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "Autotangent") {
    parameters = { brightness };
    oscillators = { brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct AutotangentShader: Shader {
  AutotangentSettings *settings;
  AutotangentShader(AutotangentSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Autotangent");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("brightness", settings->brightness->value);
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
    return ShaderTypeAutotangent;
  }

  void drawSettings() override {
    CommonViews::H3Title("Autotangent");

    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
