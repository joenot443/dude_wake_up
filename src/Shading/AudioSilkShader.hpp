//
//  AudioSilkShader.hpp
//  dude_wake_up
//
//  Draped silk with anisotropic (Kajiya-Kay) sheen from an orbiting key + rim light.
//  Each fold crest is woven to a frequency band and glows with it; kicks (relative bass
//  onsets, measured on the CPU) gust through the cloth; highs glint on the sheen; three dye palettes bleed into each other ~16 s.
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
#include <algorithm>
#include <array>
#include <cmath>

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

// CPU-side audio analysis. Kicks are relative onsets of raw-waveform bass energy against a
// seeded 4 s average: the spectrum's low bins clamp at 1 on loud masters, so an absolute
// threshold left the kick permanently on. Mids/highs are auto-gained against 8 s averages.
struct SilkCPU {
  static constexpr int HIST = 1024;
  float bassAvg = 0.0f, kickEnv = 0.0f;
  float shape[2] = {0, 0}, avg[2] = {0, 0};
  float midLevel = 0.5f, highLevel = 0.5f;

  static float clampf(float x, float a, float b) { return std::max(a, std::min(b, x)); }
  static float smoothstep(float a, float b, float x) { float u = clampf((x - a) / (b - a), 0.0f, 1.0f); return u * u * (3.0f - 2.0f * u); }
  static float band(const float *sp, float lo, float hi) {
    float s = 0.0f;
    for (int j = 0; j < 8; j++) s += sp[(int)clampf(int((lo + (hi - lo) * j / 7.0f) * 255.0f), 0, 255)];
    return s / 8.0f;
  }

  void update(float dt, const float *sp, const float *hist, float sens) {
    float energy = 0.0f;
    for (int i = 0; i < 256; i += 8) energy += sp[i];
    float live = smoothstep(0.05f, 0.4f, energy);

    double bm2 = 0.0;
    for (int b = 0; b < HIST; b += 64) {
      double m = 0.0;
      for (int i = 0; i < 64; i++) m += hist[b + i];
      m /= 64.0;
      bm2 += m * m;
    }
    float bass = float(std::sqrt(bm2 / (HIST / 64)));
    if (bassAvg <= 0.0f) bassAvg = bass;   // seed: no false onsets while the average warms up
    bassAvg += (bass - bassAvg) * (1.0f - std::exp(-dt / 4.0f));
    float onset = bass / std::max(bassAvg, 1e-3f) - 1.0f;
    float target = smoothstep(0.25f, 0.8f, onset * (0.75f + 0.25f * sens)) * live;
    // short attack (60 ms) so the gust of light ramps over a few frames instead of snapping on; 250 ms release
    if (target > kickEnv) kickEnv += (target - kickEnv) * (1.0f - std::exp(-dt / 0.06f));
    else kickEnv *= std::exp(-dt / 0.25f);

    float bands[2] = { band(sp, 0.10f, 0.45f), band(sp, 0.50f, 1.00f) };
    if (live > 0.01f && std::max(avg[0], avg[1]) <= 1e-3f)
      for (int i = 0; i < 2; i++) avg[i] = shape[i] = bands[i];   // seed on first sound
    float k = 1.0f - std::exp(-dt / 0.25f), ka = 1.0f - std::exp(-dt / 8.0f);
    float lv[2];
    for (int i = 0; i < 2; i++) {
      shape[i] += (bands[i] - shape[i]) * k;
      avg[i] = std::max(avg[i] + (shape[i] - avg[i]) * ka, 1e-3f);
      lv[i] = smoothstep(0.0f, 1.0f, 0.5f + 0.5f * (0.5f + 0.5f * sens) * (shape[i] / avg[i] - 1.0f));
    }
    midLevel = lv[0];
    highLevel = lv[1];
  }
};

struct AudioSilkShader: Shader {
  AudioSilkSettings *settings;
  AudioSilkShader(AudioSilkSettings *settings) : settings(settings), Shader(settings) {};

  // Speed integrated as accumulated phase (rate, not time*speed) so changing it
  // eases the motion instead of warping the field. Same as MarbleFlowShader.
  float speedPhase = 0.0f;
  SilkCPU cpu;
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
    static const std::array<float, 256> silentSpec = {};
    static const std::array<float, AudioAnalysis::kRawHistorySize> silentHist = {};
    const float *spec = source != nullptr ? source->audioAnalysis.renderSpectrum.data() : silentSpec.data();
    const float *hist = source != nullptr ? source->audioAnalysis.renderRawHistory.data() : silentHist.data();
    cpu.update(dt, spec, hist, settings->audioSensitivity->value);

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
    shader.setUniform1f("kickLevel", cpu.kickEnv);
    shader.setUniform1f("midLevel", cpu.midLevel);
    shader.setUniform1f("highLevel", cpu.highLevel);
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
