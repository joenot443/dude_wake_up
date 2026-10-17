//
//  AudioSuminagashiShader.hpp
//  dude_wake_up
//
//  Japanese ink marbling on dark water, built on frame feedback (the output is the
//  state). Ink commits on the beat and clear water between beats at the same site,
//  pushing earlier ink outward into concentric rings; currents, combs, swirls and
//  stones marble them; palettes re-dye the sheet along a travelling front.
//

#ifndef AudioSuminagashiShader_hpp
#define AudioSuminagashiShader_hpp

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

struct AudioSuminagashiSettings: public ShaderSettings {
  std::shared_ptr<Parameter> dropForce;
  std::shared_ptr<WaveformOscillator> dropForceOscillator;

  std::shared_ptr<Parameter> kickPunch;
  std::shared_ptr<WaveformOscillator> kickPunchOscillator;

  std::shared_ptr<Parameter> current;
  std::shared_ptr<WaveformOscillator> currentOscillator;

  std::shared_ptr<Parameter> gestureStrength;
  std::shared_ptr<WaveformOscillator> gestureStrengthOscillator;

  std::shared_ptr<Parameter> shimmer;
  std::shared_ptr<WaveformOscillator> shimmerOscillator;

  std::shared_ptr<Parameter> separation;
  std::shared_ptr<WaveformOscillator> separationOscillator;

  std::shared_ptr<Parameter> tension;
  std::shared_ptr<WaveformOscillator> tensionOscillator;

  std::shared_ptr<Parameter> vignette;
  std::shared_ptr<WaveformOscillator> vignetteOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  AudioSuminagashiSettings(std::string shaderId, json j) :
  dropForce(std::make_shared<Parameter>("Drop Force", 1.0, 0.0, 3.0)),
  dropForceOscillator(std::make_shared<WaveformOscillator>(dropForce)),
  kickPunch(std::make_shared<Parameter>("Kick Punch", 1.0, 0.0, 3.0)),
  kickPunchOscillator(std::make_shared<WaveformOscillator>(kickPunch)),
  current(std::make_shared<Parameter>("Current", 1.0, 0.0, 2.0)),
  currentOscillator(std::make_shared<WaveformOscillator>(current)),
  gestureStrength(std::make_shared<Parameter>("Gestures", 1.0, 0.0, 1.5)),
  gestureStrengthOscillator(std::make_shared<WaveformOscillator>(gestureStrength)),
  shimmer(std::make_shared<Parameter>("Shimmer", 1.0, 0.0, 3.0)),
  shimmerOscillator(std::make_shared<WaveformOscillator>(shimmer)),
  separation(std::make_shared<Parameter>("Separation", 1.0, 0.0, 2.0)),
  separationOscillator(std::make_shared<WaveformOscillator>(separation)),
  tension(std::make_shared<Parameter>("Tension", 1.0, 0.0, 1.5)),
  tensionOscillator(std::make_shared<WaveformOscillator>(tension)),
  vignette(std::make_shared<Parameter>("Vignette", 1.0, 0.0, 1.0)),
  vignetteOscillator(std::make_shared<WaveformOscillator>(vignette)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  ShaderSettings(shaderId, j, "AudioSuminagashi") {
    parameters = { dropForce, kickPunch, current, gestureStrength, shimmer, separation, tension, vignette, audioSensitivity };
    oscillators = { dropForceOscillator, kickPunchOscillator, currentOscillator, gestureStrengthOscillator, shimmerOscillator, separationOscillator, tensionOscillator, vignetteOscillator, audioSensitivityOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioSuminagashiShader: Shader {
  AudioSuminagashiSettings *settings;
  AudioSuminagashiShader(AudioSuminagashiSettings *settings) : settings(settings), Shader(settings) {};

  float lastTimeSample = 0.0f;
  bool timeSampled = false;
  // Slow running averages of each band, so onsets are detected relative to the
  // track's own levels rather than absolute thresholds.
  float bassAvg = 0.0f, midAvg = 0.0f, highAvg = 0.0f;
  bool avgSeeded = false;

  void setup() override {
    shader.load("shaders/AudioSuminagashi");
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
    static const std::array<float, 256> silence = {};
    const float *spec = source != nullptr ? source->audioAnalysis.renderSpectrum.data() : silence.data();
    float b = band(spec, 0.0f, 0.08f), m = band(spec, 0.10f, 0.45f), h = band(spec, 0.50f, 1.00f);
    if (!avgSeeded) { bassAvg = b; midAvg = m; highAvg = h; avgSeeded = true; }
    float k = 1.0f - std::exp(-dt / 4.0f);
    bassAvg += (b - bassAvg) * k;
    midAvg += (m - midAvg) * k;
    highAvg += (h - highAvg) * k;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1fv("audio", spec, 256);
    shader.setUniform1f("frameDt", dt);
    shader.setUniform1f("bassAvg", bassAvg);
    shader.setUniform1f("midAvg", midAvg);
    shader.setUniform1f("highAvg", highAvg);
    shader.setUniform1f("beatPulse", source != nullptr ? source->audioAnalysis.beatPulse->value : 0.0f);
    shader.setUniform1f("bpm", source != nullptr ? source->audioAnalysis.bpm->value : 120.0f);
    shader.setUniform1f("dropForce", settings->dropForce->value);
    shader.setUniform1f("kickPunch", settings->kickPunch->value);
    shader.setUniform1f("current", settings->current->value);
    shader.setUniform1f("gestureStrength", settings->gestureStrength->value);
    shader.setUniform1f("shimmer", settings->shimmer->value);
    shader.setUniform1f("separation", settings->separation->value);
    shader.setUniform1f("tension", settings->tension->value);
    shader.setUniform1f("vignette", settings->vignette->value);
    shader.setUniform1f("audioSensitivity", settings->audioSensitivity->value);
    shader.setUniform1f("time", now);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioSuminagashi; }

  void drawSettings() override {
    CommonViews::H3Title("AudioSuminagashi");
    CommonViews::ShaderParameter(settings->dropForce, settings->dropForceOscillator);
    CommonViews::ShaderParameter(settings->kickPunch, settings->kickPunchOscillator);
    CommonViews::ShaderParameter(settings->current, settings->currentOscillator);
    CommonViews::ShaderParameter(settings->gestureStrength, settings->gestureStrengthOscillator);
    CommonViews::ShaderParameter(settings->shimmer, settings->shimmerOscillator);
    CommonViews::ShaderParameter(settings->separation, settings->separationOscillator);
    CommonViews::ShaderParameter(settings->tension, settings->tensionOscillator);
    CommonViews::ShaderParameter(settings->vignette, settings->vignetteOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
  }
};

#endif
