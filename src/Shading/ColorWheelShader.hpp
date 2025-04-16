//
//  ColorWheelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorWheelShader_hpp
#define ColorWheelShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include "AudioSourceService.hpp"
#include <stdio.h>

struct ColorWheelSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enableAudio;
  std::shared_ptr<WaveformOscillator> enableAudioOscillator;
  
  std::shared_ptr<Parameter> ballDistance;
  std::shared_ptr<WaveformOscillator> ballDistanceOscillator;

  ColorWheelSettings(std::string shaderId, json j) :
  enableAudio(std::make_shared<Parameter>("enableAudio", 0.5, 0.0, 1.0)),
  enableAudioOscillator(std::make_shared<WaveformOscillator>(enableAudio)),
  ballDistance(std::make_shared<Parameter>("ballDistance", 1.0, 0.0, 5.0)),
  ballDistanceOscillator(std::make_shared<WaveformOscillator>(ballDistance)),
  ShaderSettings(shaderId, j, "ColorWheel") {
    parameters = { enableAudio, ballDistance };
    oscillators = { enableAudioOscillator, ballDistanceOscillator };
    load(j);
    registerParameters();
  };
};

struct ColorWheelShader: Shader {
  ColorWheelSettings *settings;
  ColorWheelShader(ColorWheelSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/ColorWheel");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (settings->enableAudio->boolValue && source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    else {
      shader.setUniform1fv("audio", &Vectors::vectorFilled(256, 1.0)[0],
                           256);
    }
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("ballDistance", settings->ballDistance->value);
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
    return ShaderTypeColorWheel;
  }

  void drawSettings() override {
    

    CommonViews::ShaderCheckbox(settings->enableAudio);
    CommonViews::ShaderParameter(settings->ballDistance, settings->ballDistanceOscillator);
  }
};

#endif
