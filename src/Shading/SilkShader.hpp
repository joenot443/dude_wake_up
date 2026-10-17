//
//  SilkShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SilkShader_hpp
#define SilkShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SilkSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> flow;
  std::shared_ptr<WaveformOscillator> flowOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> sheen;
  std::shared_ptr<WaveformOscillator> sheenOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  SilkSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scale(std::make_shared<Parameter>("Scale", 1.5, 0.3, 5.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  flow(std::make_shared<Parameter>("Flow", 0.028, 0.005, 0.08)),
  flowOscillator(std::make_shared<WaveformOscillator>(flow)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  sheen(std::make_shared<Parameter>("Sheen", 0.1, 0.0, 0.5)),
  sheenOscillator(std::make_shared<WaveformOscillator>(sheen)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.3, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "Silk") {
    parameters = { speed, scale, flow, hueShift, sheen, brightness };
    oscillators = { speedOscillator, scaleOscillator, flowOscillator,
                    hueShiftOscillator, sheenOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct SilkShader: Shader {
  SilkSettings *settings;
  SilkShader(SilkSettings *settings) : settings(settings), Shader(settings) {};

  // Speed is integrated into an accumulated phase rather than multiplying
  // absolute time, so changing Speed alters the rate going forward instead of
  // instantly warping the whole field. Same approach as IridescentShader.
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/Silk");
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
    shader.setUniform1f("flow", settings->flow->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("sheen", settings->sheen->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    // Speed-scaled accumulated phase (not raw time * speed).
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
    return ShaderTypeSilk;
  }

  void drawSettings() override {
    CommonViews::H3Title("Silk");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->flow, settings->flowOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->sheen, settings->sheenOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* SilkShader_hpp */
