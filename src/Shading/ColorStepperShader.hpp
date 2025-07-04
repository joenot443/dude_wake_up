//
//  ColorStepperShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorStepperShader_hpp
#define ColorStepperShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ColorStepperSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<Parameter> smoothness;
  std::shared_ptr<WaveformOscillator> smoothnessOscillator;
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  ColorStepperSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.1, 10.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  smoothness(std::make_shared<Parameter>("Smoothness", 0.5, 0.0, 1.0)),
  smoothnessOscillator(std::make_shared<WaveformOscillator>(smoothness)),
  amount(std::make_shared<Parameter>("Amount", 1.0, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "ColorStepper") {
    parameters = { speed, smoothness, amount };
    oscillators = { speedOscillator, smoothnessOscillator, amountOscillator };
    audioReactiveParameter = speed;
    load(j);
    registerParameters();
  };
};

struct ColorStepperShader: Shader {
  ColorStepperSettings *settings;
  ColorStepperShader(ColorStepperSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/ColorStepper");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->max - settings->speed->value);
    shader.setUniform1f("smoothness", settings->smoothness->value);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
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
    return ShaderTypeColorStepper;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->smoothness, settings->smoothnessOscillator);
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif
