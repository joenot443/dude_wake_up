//
//  UnknownPleasuresShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef UnknownPleasuresShader_hpp
#define UnknownPleasuresShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct UnknownPleasuresSettings: public ShaderSettings {
  std::shared_ptr<Parameter> height;
  std::shared_ptr<WaveformOscillator> heightOscillator;
  
  // New parameters for waveform control
  std::shared_ptr<Parameter> lineDensity;  // Controls spacing between lines
  std::shared_ptr<WaveformOscillator> lineDensityOscillator;
  
  std::shared_ptr<Parameter> noiseIntensity;  // Controls the amount of noise in the waveform
  std::shared_ptr<WaveformOscillator> noiseIntensityOscillator;
  
  std::shared_ptr<Parameter> waveFrequency;  // Controls the frequency of the base wave
  std::shared_ptr<WaveformOscillator> waveFrequencyOscillator;
  
  std::shared_ptr<Parameter> timeScale;  // Controls the speed of the animation
  std::shared_ptr<WaveformOscillator> timeScaleOscillator;

  UnknownPleasuresSettings(std::string shaderId, json j) :
    height(std::make_shared<Parameter>("Height", 1.0, 0.0, 3.0)),
    heightOscillator(std::make_shared<WaveformOscillator>(height)),
    lineDensity(std::make_shared<Parameter>("Line Density", 0.5, 0.1, 2.0)),
    lineDensityOscillator(std::make_shared<WaveformOscillator>(lineDensity)),
    noiseIntensity(std::make_shared<Parameter>("Noise Intensity", 0.5, 0.0, 1.0)),
    noiseIntensityOscillator(std::make_shared<WaveformOscillator>(noiseIntensity)),
    waveFrequency(std::make_shared<Parameter>("Wave Frequency", 1.5, 0.1, 3.0)),
    waveFrequencyOscillator(std::make_shared<WaveformOscillator>(waveFrequency)),
    timeScale(std::make_shared<Parameter>("Time Scale", 1.0, 0.1, 3.0)),
    timeScaleOscillator(std::make_shared<WaveformOscillator>(timeScale)),
    ShaderSettings(shaderId, j, "UnknownPleasures") {
      parameters = { height, lineDensity, noiseIntensity, waveFrequency, timeScale };
      oscillators = { heightOscillator, lineDensityOscillator, noiseIntensityOscillator, 
                     waveFrequencyOscillator, timeScaleOscillator };
      load(j);
      registerParameters();
    };
};

struct UnknownPleasuresShader: Shader {
  UnknownPleasuresSettings *settings;
  UnknownPleasuresShader(UnknownPleasuresSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  
  
  void setup() override {
    shader.load("shaders/UnknownPleasures");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("height", settings->height->value);
    shader.setUniform1f("lineDensity", settings->lineDensity->value);
    shader.setUniform1f("noiseIntensity", settings->noiseIntensity->value);
    shader.setUniform1f("waveFrequency", settings->waveFrequency->value);
    shader.setUniform1f("timeScale", settings->timeScale->value);
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
    return ShaderTypeUnknownPleasures;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->height, settings->heightOscillator);
    CommonViews::ShaderParameter(settings->lineDensity, settings->lineDensityOscillator);
    CommonViews::ShaderParameter(settings->noiseIntensity, settings->noiseIntensityOscillator);
    CommonViews::ShaderParameter(settings->waveFrequency, settings->waveFrequencyOscillator);
    CommonViews::ShaderParameter(settings->timeScale, settings->timeScaleOscillator);
  }
};

#endif
