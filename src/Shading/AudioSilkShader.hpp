//
//  AudioSilkShader.hpp
//  dude_wake_up
//
//  Draped silk with anisotropic (Kajiya-Kay) sheen from an orbiting key + rim light.
//  Each fold crest is woven to a frequency band and glows with it; kicks gust through
//  the cloth; highs glint on the sheen; three dye palettes bleed into each other ~16 s.
//

#ifndef AudioSilkShader_hpp
#define AudioSilkShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioSilkSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> ripple;
  std::shared_ptr<WaveformOscillator> rippleOscillator;

  std::shared_ptr<Parameter> sheen;
  std::shared_ptr<WaveformOscillator> sheenOscillator;

  std::shared_ptr<Parameter> gust;
  std::shared_ptr<WaveformOscillator> gustOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AudioSilkSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scale(std::make_shared<Parameter>("Scale", 1.1, 0.4, 3.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  ripple(std::make_shared<Parameter>("Ripple", 0.55, 0.0, 1.2)),
  rippleOscillator(std::make_shared<WaveformOscillator>(ripple)),
  sheen(std::make_shared<Parameter>("Sheen", 1.0, 0.0, 3.0)),
  sheenOscillator(std::make_shared<WaveformOscillator>(sheen)),
  gust(std::make_shared<Parameter>("Gust", 1.0, 0.0, 3.0)),
  gustOscillator(std::make_shared<WaveformOscillator>(gust)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.2, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "AudioSilk") {
    parameters = { speed, scale, ripple, sheen, gust, hueShift, audioSensitivity, brightness };
    oscillators = { speedOscillator, scaleOscillator, rippleOscillator, sheenOscillator, gustOscillator, hueShiftOscillator, audioSensitivityOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioSilkShader: Shader {
  AudioSilkSettings *settings;
  AudioSilkShader(AudioSilkSettings *settings) : settings(settings), Shader(settings) {};

  // Speed integrated as accumulated phase (rate, not time*speed) so changing it
  // eases the motion instead of warping the field. Same as MarbleFlowShader.
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/AudioSilk");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;
    speedPhase += dt * settings->speed->value;

    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("ripple", settings->ripple->value);
    shader.setUniform1f("sheen", settings->sheen->value);
    shader.setUniform1f("gust", settings->gust->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("audioSensitivity", settings->audioSensitivity->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", speedPhase);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioSilk; }

  void drawSettings() override {
    CommonViews::H3Title("AudioSilk");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->ripple, settings->rippleOscillator);
    CommonViews::ShaderParameter(settings->sheen, settings->sheenOscillator);
    CommonViews::ShaderParameter(settings->gust, settings->gustOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
