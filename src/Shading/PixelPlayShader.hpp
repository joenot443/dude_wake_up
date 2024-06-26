//
//  PixelPlayShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelPlayShader_hpp
#define PixelPlayShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PixelPlaySettings: public ShaderSettings {
  std::shared_ptr<Parameter> displaceFactor;
  std::shared_ptr<ValueOscillator> displaceFactorOscillator;

  PixelPlaySettings(std::string shaderId, json j) :
  displaceFactor(std::make_shared<Parameter>("Displace Factor", 0.5, 0.0, 1.0)),
  displaceFactorOscillator(std::make_shared<ValueOscillator>(displaceFactor)),
  ShaderSettings(shaderId, j, "PixelPlay") {
    parameters = { displaceFactor };
    oscillators = { displaceFactorOscillator };
    load(j);
    registerParameters();
  };
};

struct PixelPlayShader: Shader {
  PixelPlaySettings *settings;
  PixelPlayShader(PixelPlaySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/PixelPlay");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("displaceFactor", settings->displaceFactor->value);
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
    return ShaderTypePixelPlay;
  }

  void drawSettings() override {
    CommonViews::H3Title("PixelPlay");

    CommonViews::ShaderParameter(settings->displaceFactor, settings->displaceFactorOscillator);
  }
};

#endif /* PixelPlayShader_hpp */
