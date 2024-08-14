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
  std::shared_ptr<WaveformOscillator> displaceFactorOscillator;
  
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  PixelPlaySettings(std::string shaderId, json j) :
  displaceFactor(std::make_shared<Parameter>("Displace Factor", 0.5, 0.0, 1.0)),
  displaceFactorOscillator(std::make_shared<WaveformOscillator>(displaceFactor)),
  amount(std::make_shared<Parameter>("Amount", 0.5, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "PixelPlay") {
    parameters = { displaceFactor };
    oscillators = { displaceFactorOscillator };
    load(j);
    audioReactiveParameter = amount;
    registerParameters();
  };
};

struct PixelPlayShader: Shader {
  PixelPlaySettings *settings;
  PixelPlayShader(PixelPlaySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/PixelPlay");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("displaceFactor", settings->displaceFactor->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("amount", settings->amount->value);
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
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* PixelPlayShader_hpp */
