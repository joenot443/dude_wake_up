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

  AudioOscillatorSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "AudioOscillator") {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
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
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() >= 256)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0], 256);
    if (source != nullptr && source->audioAnalysis.smoothWaveform.size() >= 256)
      shader.setUniform1fv("waveform", &source->audioAnalysis.smoothWaveform[0], 256);
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

  }
};

#endif /* AudioOscillatorShader_hpp */
