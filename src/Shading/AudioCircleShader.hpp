//
//  AudioCircleShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioCircleShader_hpp
#define AudioCircleShader_hpp

#include "ofMain.h"
#include "AudioSourceService.hpp"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioCircleSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<ValueOscillator> shaderValueOscillator;

  AudioCircleSettings(std::string shaderId, json j, std::string name) : shaderValue(std::make_shared<Parameter>("shaderValue", 1.0, -1.0, 2.0)),
                                                      shaderValueOscillator(std::make_shared<ValueOscillator>(shaderValue)),
                                                      ShaderSettings(shaderId, j, name) 
  {
    parameters = {shaderValue};
    oscillators = {shaderValueOscillator};
    load(j);
    registerParameters();
  };
};

struct AudioCircleShader : Shader
{
  AudioCircleSettings *settings;
  AudioCircleShader(AudioCircleSettings *settings) : settings(settings), Shader(settings){};

  void setup() override
  {
    shader.load("shaders/AudioCircle");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeAudioCircle;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("AudioCircle");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif
