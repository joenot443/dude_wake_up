//
//  LimboShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LimboShader_hpp
#define LimboShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct LimboSettings: public ShaderSettings {
  std::shared_ptr<Parameter> colorWarp;
  std::shared_ptr<ValueOscillator> colorWarpOscillator;

  LimboSettings(std::string shaderId, json j) :
  colorWarp(std::make_shared<Parameter>("colorWarp", 1.0, -1.0, 2.0)),
  colorWarpOscillator(std::make_shared<ValueOscillator>(colorWarp)),
  ShaderSettings(shaderId, j) {
    parameters = { colorWarp };
    oscillators = { colorWarpOscillator };
    load(j);
    registerParameters();
  };
};

struct LimboShader: Shader {
  LimboSettings *settings;
  LimboShader(LimboSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Limbo");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("colorWarp", settings->colorWarp->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeLimbo;
  }

  void drawSettings() override {
    CommonViews::H3Title("Limbo");

    CommonViews::ShaderParameter(settings->colorWarp, settings->colorWarpOscillator);
  }
};

#endif /* LimboShader_hpp */