//
//  GemfieldShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GemfieldShader_hpp
#define GemfieldShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GemfieldSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> seamGlow;
  std::shared_ptr<WaveformOscillator> seamGlowOscillator;

  std::shared_ptr<Parameter> ripple;
  std::shared_ptr<WaveformOscillator> rippleOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  GemfieldSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scale(std::make_shared<Parameter>("Scale", 2.5, 1.0, 6.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  seamGlow(std::make_shared<Parameter>("Seam Glow", 0.9, 0.0, 2.0)),
  seamGlowOscillator(std::make_shared<WaveformOscillator>(seamGlow)),
  ripple(std::make_shared<Parameter>("Ripple", 1.0, 0.0, 2.0)),
  rippleOscillator(std::make_shared<WaveformOscillator>(ripple)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  brightness(std::make_shared<Parameter>("Brightness", 1.3, 0.3, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "Gemfield") {
    parameters = { speed, scale, seamGlow, ripple, hueShift, brightness };
    oscillators = { speedOscillator, scaleOscillator, seamGlowOscillator,
                    rippleOscillator, hueShiftOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct GemfieldShader: Shader {
  GemfieldSettings *settings;
  GemfieldShader(GemfieldSettings *settings) : settings(settings), Shader(settings) {};

  // Speed integrated as accumulated phase (rate, not time*speed) so changing it
  // eases the cell motion instead of warping the field. Same as IridescentShader.
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/Gemfield");
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
    shader.setUniform1f("seamGlow", settings->seamGlow->value);
    shader.setUniform1f("ripple", settings->ripple->value);
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
    return ShaderTypeGemfield;
  }

  void drawSettings() override {
    CommonViews::H3Title("Gemfield");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->seamGlow, settings->seamGlowOscillator);
    CommonViews::ShaderParameter(settings->ripple, settings->rippleOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* GemfieldShader_hpp */
