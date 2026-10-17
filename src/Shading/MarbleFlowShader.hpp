//
//  MarbleFlowShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MarbleFlowShader_hpp
#define MarbleFlowShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MarbleFlowSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> warp;
  std::shared_ptr<WaveformOscillator> warpOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  MarbleFlowSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scale(std::make_shared<Parameter>("Scale", 1.5, 0.4, 5.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  warp(std::make_shared<Parameter>("Warp", 1.0, 0.0, 2.5)),
  warpOscillator(std::make_shared<WaveformOscillator>(warp)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.3, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "MarbleFlow") {
    parameters = { speed, scale, warp, hueShift, brightness };
    oscillators = { speedOscillator, scaleOscillator, warpOscillator,
                    hueShiftOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct MarbleFlowShader: Shader {
  MarbleFlowSettings *settings;
  MarbleFlowShader(MarbleFlowSettings *settings) : settings(settings), Shader(settings) {};

  // Speed integrated as accumulated phase (rate, not time*speed) so changing it
  // eases the flow instead of warping the field. Same as IridescentShader.
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/MarbleFlow");
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
    shader.setUniform1f("hueShift", settings->hueShift->value);
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
    return ShaderTypeMarbleFlow;
  }

  void drawSettings() override {
    CommonViews::H3Title("MarbleFlow");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->warp, settings->warpOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* MarbleFlowShader_hpp */
