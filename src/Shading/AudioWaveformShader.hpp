//
//  AudioWaveformShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioWaveformShader_hpp
#define AudioWaveformShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "AudioSourceService.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioWaveformSettings: public ShaderSettings {
  std::string shaderId;
  
  AudioWaveformSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  ShaderSettings(shaderId) {
    parameters = {};
    oscillators = {};
    load(j);
  };
};

struct AudioWaveformShader: Shader {
  AudioWaveformSettings *settings;
  AudioWaveformShader(AudioWaveformSettings *settings) :
  settings(settings),
  Shader(settings) {};


  ofShader shader;
  void setup() override {
    shader.load("../../shaders/AudioWaveform");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0], 256);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {

  }

  ShaderType type() override {
    return ShaderTypeAudioWaveform;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioWaveform");
  }
};

#endif /* AudioWaveformShader_hpp */
