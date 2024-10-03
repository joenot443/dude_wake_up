//
//  FlickerAudioShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FlickerAudioShader_hpp
#define FlickerAudioShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "AudioSourceService.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FlickerAudioSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enableAudio;

  FlickerAudioSettings(std::string shaderId, json j) :
  enableAudio(std::make_shared<Parameter>("Enable Audio", 0.5, 0.0, 1.0)),
  ShaderSettings(shaderId, j, "FlickerAudio") {
    parameters = { enableAudio };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct FlickerAudioShader: Shader {
  FlickerAudioSettings *settings;
  FlickerAudioShader(FlickerAudioSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/FlickerAudio");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (settings->enableAudio->boolValue && source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("enableAudio", settings->enableAudio->value);
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
    return ShaderTypeFlickerAudio;
  }

  void drawSettings() override {
    CommonViews::H3Title("FlickerAudio");

    CommonViews::ShaderCheckbox(settings->enableAudio);
  }
};

#endif /* FlickerAudioShader_hpp */
