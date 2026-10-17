//
//  AudioPrismShader.hpp
//  dude_wake_up
//
//  The audio spectrum becomes the light spectrum: a white beam refracts through a
//  glass prism (real Snell's law) into a fan that lands on a card as a live
//  spectrograph — each wavelength's brightness is a frequency band, quiet bands show
//  as absorption lines. Kicks open the fan; epochs swing the bench or eclipse the beam.
//

#ifndef AudioPrismShader_hpp
#define AudioPrismShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include <cmath>

struct AudioPrismSettings: public ShaderSettings {
  std::shared_ptr<Parameter> beamPower;
  std::shared_ptr<WaveformOscillator> beamPowerOscillator;

  std::shared_ptr<Parameter> dispersion;
  std::shared_ptr<WaveformOscillator> dispersionOscillator;

  std::shared_ptr<Parameter> haze;
  std::shared_ptr<WaveformOscillator> hazeOscillator;

  std::shared_ptr<Parameter> glass;
  std::shared_ptr<WaveformOscillator> glassOscillator;

  std::shared_ptr<Parameter> floorGloss;
  std::shared_ptr<WaveformOscillator> floorGlossOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AudioPrismSettings(std::string shaderId, json j) :
  beamPower(std::make_shared<Parameter>("Beam Power", 1.0, 0.0, 3.0)),
  beamPowerOscillator(std::make_shared<WaveformOscillator>(beamPower)),
  dispersion(std::make_shared<Parameter>("Dispersion", 1.0, 0.0, 1.5)),
  dispersionOscillator(std::make_shared<WaveformOscillator>(dispersion)),
  haze(std::make_shared<Parameter>("Haze", 1.0, 0.0, 2.0)),
  hazeOscillator(std::make_shared<WaveformOscillator>(haze)),
  glass(std::make_shared<Parameter>("Glass", 1.0, 0.0, 2.0)),
  glassOscillator(std::make_shared<WaveformOscillator>(glass)),
  floorGloss(std::make_shared<Parameter>("Floor Gloss", 1.0, 0.0, 1.5)),
  floorGlossOscillator(std::make_shared<WaveformOscillator>(floorGloss)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.4, 0.2, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "AudioPrism") {
    parameters = { beamPower, dispersion, haze, glass, floorGloss, hueShift, audioSensitivity, brightness };
    oscillators = { beamPowerOscillator, dispersionOscillator, hazeOscillator, glassOscillator, floorGlossOscillator, hueShiftOscillator, audioSensitivityOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioPrismShader: Shader {
  AudioPrismSettings *settings;
  AudioPrismShader(AudioPrismSettings *settings) : settings(settings), Shader(settings) {};

  float lastTimeSample = 0.0f;
  bool timeSampled = false;
  // Slow running average of the bass band, so kicks are detected relative to the
  // track's own level (brickwalled masters would otherwise pin the kick on).
  float bassAvg = 0.0f;
  bool bassAvgSeeded = false;

  void setup() override {
    shader.load("shaders/AudioPrism");
  }

  // Mean of 8 bins across [lo, hi] — identical to the shader's band().
  static float band(const float *spec, float lo, float hi) {
    float s = 0.0f;
    for (int j = 0; j < 8; j++) {
      int i = std::clamp(int((lo + (hi - lo) * j / 7.0f) * 255.0f), 0, 255);
      s += spec[i];
    }
    return s / 8.0f;
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;

    auto source = AudioSourceService::getService()->selectedAudioSource;
    float bass = source != nullptr ? band(source->audioAnalysis.renderSpectrum.data(), 0.0f, 0.08f) : 0.0f;
    if (!bassAvgSeeded) { bassAvg = bass; bassAvgSeeded = true; }
    bassAvg += (bass - bassAvg) * (1.0f - std::exp(-dt / 4.0f));

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1f("frameDt", dt);
    shader.setUniform1f("bassAvg", bassAvg);
    shader.setUniform1f("beamPower", settings->beamPower->value);
    shader.setUniform1f("dispersion", settings->dispersion->value);
    shader.setUniform1f("haze", settings->haze->value);
    shader.setUniform1f("glass", settings->glass->value);
    shader.setUniform1f("floorGloss", settings->floorGloss->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("audioSensitivity", settings->audioSensitivity->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", now);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioPrism; }

  void drawSettings() override {
    CommonViews::H3Title("AudioPrism");
    CommonViews::ShaderParameter(settings->beamPower, settings->beamPowerOscillator);
    CommonViews::ShaderParameter(settings->dispersion, settings->dispersionOscillator);
    CommonViews::ShaderParameter(settings->haze, settings->hazeOscillator);
    CommonViews::ShaderParameter(settings->glass, settings->glassOscillator);
    CommonViews::ShaderParameter(settings->floorGloss, settings->floorGlossOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
