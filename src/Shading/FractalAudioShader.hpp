//
//  FractalAudioShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FractalAudioShader_hpp
#define FractalAudioShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "AudioSourceService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Vectors.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FractalAudioSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enableAudio;
  std::shared_ptr<WaveformOscillator> enableAudioOscillator;
  
  FractalAudioSettings(std::string shaderId, json j) :
  enableAudio(std::make_shared<Parameter>("Enable Audio", 0.5, 0.0, 1.0)),
  enableAudioOscillator(std::make_shared<WaveformOscillator>(enableAudio)),
  ShaderSettings(shaderId, j, "FractalAudio") {
    parameters = { enableAudio };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct FractalAudioShader: Shader {
  FractalAudioSettings *settings;
  FractalAudioShader(FractalAudioSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/FractalAudio");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (settings->enableAudio->boolValue && source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    else {
      shader.setUniform1fv("audio", &Vectors::vectorFilled(256, 0.0)[0],
                           256);
    }
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
    return ShaderTypeFractalAudio;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("FractalAudio");
    
    CommonViews::ShaderCheckbox(settings->enableAudio);
  }
};

#endif /* FractalAudioShader_hpp */
