//
//  AuroraShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2024.
//

#ifndef AuroraShader_hpp
#define AuroraShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AuroraSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> intensity;
  std::shared_ptr<Parameter> waveHeight;
  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<Parameter> bandCount;
  std::shared_ptr<Parameter> noiseScale;

  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> intensityOscillator;
  std::shared_ptr<WaveformOscillator> waveHeightOscillator;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;
  std::shared_ptr<WaveformOscillator> bandCountOscillator;
  std::shared_ptr<WaveformOscillator> noiseScaleOscillator;

  AuroraSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  intensity(std::make_shared<Parameter>("Intensity", 0.8, 0.0, 2.0)),
  waveHeight(std::make_shared<Parameter>("Wave Height", 0.15, 0.0, 0.5)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, 1.0)),
  bandCount(std::make_shared<Parameter>("Band Count", 3.0, 1.0, 4.0)),
  noiseScale(std::make_shared<Parameter>("Noise Scale", 3.0, 0.5, 8.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  intensityOscillator(std::make_shared<WaveformOscillator>(intensity)),
  waveHeightOscillator(std::make_shared<WaveformOscillator>(waveHeight)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  bandCountOscillator(std::make_shared<WaveformOscillator>(bandCount)),
  noiseScaleOscillator(std::make_shared<WaveformOscillator>(noiseScale)),
  ShaderSettings(shaderId, j, "Aurora") {
    parameters = { speed, intensity, waveHeight, colorShift, bandCount, noiseScale };
    oscillators = { speedOscillator, intensityOscillator, waveHeightOscillator, colorShiftOscillator, bandCountOscillator, noiseScaleOscillator };
    load(j);
    registerParameters();
  };
};

struct AuroraShader: Shader {
  AuroraSettings *settings;
  AuroraShader(AuroraSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Aurora");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("intensity", settings->intensity->value);
    shader.setUniform1f("waveHeight", settings->waveHeight->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
    shader.setUniform1f("bandCount", settings->bandCount->value);
    shader.setUniform1f("noiseScale", settings->noiseScale->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeAurora;
  }

  void drawSettings() override {
    CommonViews::H3Title("Aurora");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->intensity, settings->intensityOscillator);
    CommonViews::ShaderParameter(settings->waveHeight, settings->waveHeightOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->bandCount, settings->bandCountOscillator);
    CommonViews::ShaderParameter(settings->noiseScale, settings->noiseScaleOscillator);
  }
};

#endif /* AuroraShader_hpp */
