//
//  UnknownPleasuresShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef UnknownPleasuresShader_hpp
#define UnknownPleasuresShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct UnknownPleasuresSettings: public ShaderSettings {
  std::shared_ptr<Parameter> height;
  std::shared_ptr<WaveformOscillator> heightOscillator;

  UnknownPleasuresSettings(std::string shaderId, json j) :
  height(std::make_shared<Parameter>("height", 0.5, 0.0, 1.0)),
  heightOscillator(std::make_shared<WaveformOscillator>(height)),
  ShaderSettings(shaderId, j, "UnknownPleasures") {
    parameters = { height };
    oscillators = { heightOscillator };
    load(j);
    registerParameters();
  };
};

struct UnknownPleasuresShader: Shader {
  UnknownPleasuresSettings *settings;
  UnknownPleasuresShader(UnknownPleasuresSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/UnknownPleasures");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("height", settings->height->value);
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
    return ShaderTypeUnknownPleasures;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->height, settings->heightOscillator);
  }
};

#endif
