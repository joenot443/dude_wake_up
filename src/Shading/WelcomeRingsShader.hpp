//
//  WelcomeRingsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WelcomeRingsShader_hpp
#define WelcomeRingsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WelcomeRingsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> cycles;
  std::shared_ptr<WaveformOscillator> cyclesOscillator;
  
  std::shared_ptr<Parameter> palette;
  std::shared_ptr<Parameter> zoom;
  std::shared_ptr<Parameter> ringFrequency;
  std::shared_ptr<Parameter> glowIntensity;
  std::shared_ptr<Parameter> glowSharpness;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> zoomOscillator;
  std::shared_ptr<WaveformOscillator> ringFrequencyOscillator;
  std::shared_ptr<WaveformOscillator> glowIntensityOscillator;
  std::shared_ptr<WaveformOscillator> glowSharpnessOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  WelcomeRingsSettings(std::string shaderId, json j) :
  cycles(std::make_shared<Parameter>("Cycles", 4.0, 1.0, 8.0)),
  cyclesOscillator(std::make_shared<WaveformOscillator>(cycles)),
  palette(std::make_shared<Parameter>("Palette", 0.0, 0.0, 7.0, ParameterType_Int)),
  zoom(std::make_shared<Parameter>("Zoom", 1.5, 0.5, 4.0)),
  ringFrequency(std::make_shared<Parameter>("Ring Frequency", 8.0, 1.0, 20.0)),
  glowIntensity(std::make_shared<Parameter>("Glow Intensity", 0.01, 0.001, 0.1)),
  glowSharpness(std::make_shared<Parameter>("Glow Sharpness", 1.2, 0.5, 3.0)),
  speed(std::make_shared<Parameter>("Speed", 0.4, 0.0, 2.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  ringFrequencyOscillator(std::make_shared<WaveformOscillator>(ringFrequency)),
  glowIntensityOscillator(std::make_shared<WaveformOscillator>(glowIntensity)),
  glowSharpnessOscillator(std::make_shared<WaveformOscillator>(glowSharpness)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j, "WelcomeRings") {
    parameters = { cycles, zoom, ringFrequency, glowIntensity, glowSharpness, speed };
    oscillators = { cyclesOscillator, zoomOscillator, ringFrequencyOscillator, glowIntensityOscillator, glowSharpnessOscillator, speedOscillator };
    audioReactiveParameter = cycles;
    load(j);
    registerParameters();
  };
};

struct WelcomeRingsShader: Shader {
  WelcomeRingsSettings *settings;
  WelcomeRingsShader(WelcomeRingsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/WelcomeRings");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("cycles", settings->cycles->value);
    shader.setUniform1i("palette", settings->palette->intValue);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform1f("ringFrequency", settings->ringFrequency->value);
    shader.setUniform1f("glowIntensity", settings->glowIntensity->value);
    shader.setUniform1f("glowSharpness", settings->glowSharpness->value);
    shader.setUniform1f("speed", settings->speed->value);
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
    return ShaderTypeWelcomeRings;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->cycles, settings->cyclesOscillator);
    CommonViews::ShaderIntParameter(settings->palette);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderParameter(settings->ringFrequency, settings->ringFrequencyOscillator);
    CommonViews::ShaderParameter(settings->glowIntensity, settings->glowIntensityOscillator);
    CommonViews::ShaderParameter(settings->glowSharpness, settings->glowSharpnessOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif
