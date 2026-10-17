//
//  FerroShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FerroShader_hpp
#define FerroShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FerroSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> warp;
  std::shared_ptr<WaveformOscillator> warpOscillator;

  std::shared_ptr<Parameter> detailAmount;
  std::shared_ptr<WaveformOscillator> detailAmountOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<WaveformOscillator> saturationOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  FerroSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scale(std::make_shared<Parameter>("Scale", 1.2, 0.5, 3.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  warp(std::make_shared<Parameter>("Warp", 1.0, 0.0, 2.0)),
  warpOscillator(std::make_shared<WaveformOscillator>(warp)),
  detailAmount(std::make_shared<Parameter>("Detail", 1.0, 0.0, 2.0)),
  detailAmountOscillator(std::make_shared<WaveformOscillator>(detailAmount)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  saturation(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 2.0)),
  saturationOscillator(std::make_shared<WaveformOscillator>(saturation)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.3, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "Ferro") {
    parameters = { speed, scale, warp, detailAmount, hueShift, saturation, brightness };
    oscillators = { speedOscillator, scaleOscillator, warpOscillator,
                    detailAmountOscillator, hueShiftOscillator,
                    saturationOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct FerroShader: Shader {
  FerroSettings *settings;
  FerroShader(FerroSettings *settings) : settings(settings), Shader(settings) {};

  // Speed integrated as accumulated phase (rate, not time*speed) so changing it
  // eases the flow instead of warping the field. Same as MarbleFlowShader.
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/Ferro");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;
    speedPhase += dt * settings->speed->value;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("warp", settings->warp->value);
    shader.setUniform1f("detailAmount", settings->detailAmount->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("saturation", settings->saturation->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", speedPhase);
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
    return ShaderTypeFerro;
  }

  void drawSettings() override {
    CommonViews::H3Title("Ferro");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->warp, settings->warpOscillator);
    CommonViews::ShaderParameter(settings->detailAmount, settings->detailAmountOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* FerroShader_hpp */
