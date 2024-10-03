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
  std::shared_ptr<Parameter> color;
  std::shared_ptr<WaveformOscillator> colorOscillator;

  MistSettings(std::string shaderId, json j) :
  color(std::make_shared<Parameter>("color", 0.5, 0.0, 1.0)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  ShaderSettings(shaderId, j, "Mist") {
    parameters = { color };
    oscillators = { colorOscillator };
    load(j);
    registerParameters();
  };
};

struct MistShader: Shader {
  MistSettings *settings;
  MistShader(MistSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Mist");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->color->value);
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
    CommonViews::H3Title("Mist");

    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
  }
};

#endif /* MistShader_hpp */
