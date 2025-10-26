//
//  AudioOscillatorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioOscillatorShader_hpp
#define AudioOscillatorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "AudioSourceService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioOscillatorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> baseColor;
  std::shared_ptr<Parameter> outlineColor;
  std::shared_ptr<Parameter> outlineThickness;
  std::shared_ptr<WaveformOscillator> outlineThicknessOscillator;

  AudioOscillatorSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  baseColor(std::make_shared<Parameter>("Base Color", ParameterType_Color)),
  outlineColor(std::make_shared<Parameter>("Outline Color", ParameterType_Color)),
  outlineThickness(std::make_shared<Parameter>("Outline Thickness", 2.0, 0.0, 10.0)),
  outlineThicknessOscillator(std::make_shared<WaveformOscillator>(outlineThickness)),
  ShaderSettings(shaderId, j, "AudioOscillator") {
    baseColor->setColor({1.0, 1.0, 1.0, 1.0});
    outlineColor->setColor({0.0, 1.0, 1.0, 1.0});
    parameters = { shaderValue, baseColor, outlineColor, outlineThickness };
    oscillators = { shaderValueOscillator, outlineThicknessOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioOscillatorShader: Shader {
  AudioOscillatorSettings *settings;
  AudioOscillatorShader(AudioOscillatorSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/AudioOscillator");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform3f("baseColor",
                        settings->baseColor->color->data()[0],
                        settings->baseColor->color->data()[1],
                        settings->baseColor->color->data()[2]);
    shader.setUniform3f("outlineColor",
                        settings->outlineColor->color->data()[0],
                        settings->outlineColor->color->data()[1],
                        settings->outlineColor->color->data()[2]);
    shader.setUniform1f("outlineThickness", settings->outlineThickness->value);
    if (source != nullptr) {
      setAudioUniform(&source->audioAnalysis.smoothWaveform);
    }
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
    return ShaderTypeAudioOscillator;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioOscillator");
    CommonViews::ShaderColor(settings->baseColor);
    CommonViews::ShaderColor(settings->outlineColor);
    CommonViews::ShaderParameter(settings->outlineThickness, settings->outlineThicknessOscillator);
  }
};

#endif /* AudioOscillatorShader_hpp */
