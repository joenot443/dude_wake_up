//
//  AudioPendulumShader.hpp
//  dude_wake_up
//
//  A pendulum-wave kinetic sculpture in a gallery. A row of polished brass/chrome spheres
//  hangs on fine wires from a dark beam; each pendulum swings (K + i) times per cycle, so the
//  row runs through travelling waves, snakes, apparent chaos and a full re-alignment every
//  cycle. Ray traced analytically (spheres, wires, beam, glossy floor, one reflection bounce,
//  soft sphere shadows, depth of field, light haze). Lighting crossfades between a warm
//  gallery, a cold blue night and a single hard theatre spotlight; the camera eases between
//  three framings.
//
//  The physics runs on the CPU (PendulumCPU below, ported 1:1 from the reference used to tune
//  the shader offline): kicks add swing VELOCITY to a damped amplitude, and the pattern clock
//  is integrated, so no angle ever jumps between frames.
//

#ifndef AudioPendulumShader_hpp
#define AudioPendulumShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include <array>
#include <cmath>

struct AudioPendulumSettings: public ShaderSettings {
  std::shared_ptr<Parameter> swing;
  std::shared_ptr<WaveformOscillator> swingOscillator;

  std::shared_ptr<Parameter> kickPunch;
  std::shared_ptr<WaveformOscillator> kickPunchOscillator;

  std::shared_ptr<Parameter> pendulums;

  std::shared_ptr<Parameter> cameraDrift;
  std::shared_ptr<WaveformOscillator> cameraDriftOscillator;

  std::shared_ptr<Parameter> keyLight;
  std::shared_ptr<WaveformOscillator> keyLightOscillator;

  std::shared_ptr<Parameter> depthOfField;
  std::shared_ptr<WaveformOscillator> depthOfFieldOscillator;

  std::shared_ptr<Parameter> haze;
  std::shared_ptr<WaveformOscillator> hazeOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AudioPendulumSettings(std::string shaderId, json j) :
  swing(std::make_shared<Parameter>("Swing", 1.0, 0.0, 2.0)),
  swingOscillator(std::make_shared<WaveformOscillator>(swing)),
  kickPunch(std::make_shared<Parameter>("Kick Punch", 1.0, 0.0, 2.0)),
  kickPunchOscillator(std::make_shared<WaveformOscillator>(kickPunch)),
  pendulums(std::make_shared<Parameter>("Pendulums", 15.0, 6.0, 20.0, ParameterType_Int)),
  cameraDrift(std::make_shared<Parameter>("Camera Drift", 1.0, 0.0, 3.0)),
  cameraDriftOscillator(std::make_shared<WaveformOscillator>(cameraDrift)),
  keyLight(std::make_shared<Parameter>("Key Light", 1.0, 0.0, 2.0)),
  keyLightOscillator(std::make_shared<WaveformOscillator>(keyLight)),
  depthOfField(std::make_shared<Parameter>("Depth Of Field", 1.0, 0.0, 3.0)),
  depthOfFieldOscillator(std::make_shared<WaveformOscillator>(depthOfField)),
  haze(std::make_shared<Parameter>("Haze", 1.0, 0.0, 3.0)),
  hazeOscillator(std::make_shared<WaveformOscillator>(haze)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.2, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "AudioPendulum") {
    parameters = { swing, kickPunch, pendulums, cameraDrift, keyLight, depthOfField, haze, hueShift, audioSensitivity, brightness };
    oscillators = { swingOscillator, kickPunchOscillator, cameraDriftOscillator, keyLightOscillator, depthOfFieldOscillator, hazeOscillator, hueShiftOscillator, audioSensitivityOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

// CPU-side pendulum-wave physics and audio analysis. One per shader instance.
struct PendulumCPU {
  static constexpr int NMAX = 20, HIST = 1024;
  static constexpr float TAU = 6.2831853f;
  static constexpr float K = 40.0f;   // slowest pendulum: K swings per cycle
  static constexpr float T = 60.0f;   // clock-seconds per full pattern cycle (re-alignment)

  double tau = 0.0;
  float clock = 1.0f;
  float amp = 0.30f, ampV = 0.0f;
  float kickEnv = 0.0f, bassAvg = 0.0f, lastKick = -10.0f;
  float shape[3] = {0, 0, 0}, avg[3] = {0, 0, 0};
  float hiFast = 0.0f, hiAvg = 0.0f, spark = 0.0f;
  float warm = 0.5f, orbit = 0.0f;

  // outputs
  std::array<float, NMAX> angles{}, lengths{};
  float count = 15.0f, align = 1.0f, live = 0.0f;

  static float clampf(float x, float a, float b) { return std::max(a, std::min(b, x)); }
  static float smoothstep(float a, float b, float x) { float u = clampf((x - a) / (b - a), 0.0f, 1.0f); return u * u * (3.0f - 2.0f * u); }
  static float band(const float *sp, float lo, float hi) {
    float s = 0.0f;
    for (int j = 0; j < 8; j++) s += sp[(int)clampf(int((lo + (hi - lo) * j / 7.0f) * 255.0f), 0, 255)];
    return s / 8.0f;
  }

  void update(float t, float dt, const float *sp, const float *hist, float sens, float swing, float kickPunch, float countKnob) {
    float energy = 0.0f;
    for (int i = 0; i < 256; i += 8) energy += sp[i];
    live = smoothstep(0.05f, 0.4f, energy);
    float bands[3] = { band(sp, 0.0f, 0.06f), band(sp, 0.06f, 0.30f), band(sp, 0.30f, 1.0f) };
    if (live > 0.01f && std::max(avg[0], std::max(avg[1], avg[2])) <= 1e-3f)
      for (int i = 0; i < 3; i++) avg[i] = shape[i] = bands[i];   // seed on first sound
    float k = 1.0f - std::exp(-dt / 0.6f), ka = 1.0f - std::exp(-dt / 8.0f);
    for (int i = 0; i < 3; i++) {
      shape[i] += (bands[i] - shape[i]) * k;
      avg[i] = std::max(avg[i] + (shape[i] - avg[i]) * ka, 1e-3f);
    }
    float g = 0.5f + 0.5f * sens;
    float sh[3];
    for (int i = 0; i < 3; i++) sh[i] = smoothstep(0.0f, 1.0f, 0.5f + 0.5f * g * (shape[i] / avg[i] - 1.0f)) * live;

    // kick: relative onset of raw-waveform bass energy (the spectrum clamps on loud masters)
    double bm2 = 0.0;
    for (int b = 0; b < HIST; b += 64) {
      double m = 0.0;
      for (int i = 0; i < 64; i++) m += hist[b + i];
      m /= 64.0;
      bm2 += m * m;
    }
    float bass = float(std::sqrt(bm2 / (HIST / 64)));
    if (bassAvg <= 0.0f) bassAvg = bass;
    bassAvg += (bass - bassAvg) * (1.0f - std::exp(-dt / 4.0f));
    float kickPrev = kickEnv;
    float onset = bass / std::max(bassAvg, 1e-3f) - 1.0f;
    float target = smoothstep(0.25f, 0.8f, onset * (0.75f + 0.25f * sens)) * live;
    kickEnv = std::max(target, kickEnv * std::exp(-dt / 0.18f));
    if (kickEnv > 0.5f && kickPrev <= 0.5f && t - lastKick > 0.3f) {
      lastKick = t;
      ampV += 0.55f * kickPunch * swing;   // an impulse: adds swing velocity
    }

    // amplitude: damped spring toward a bass-driven target (silence keeps a gentle swing)
    float tgt = swing * (0.30f + 0.24f * sh[0]);
    float h = dt / 4.0f;
    for (int s = 0; s < 4; s++) {
      float acc = -5.0f * (amp - tgt) - 3.2f * ampV;
      ampV += acc * h;
      amp += ampV * h;
    }
    amp = clampf(amp, 0.0f, 1.0f);

    // the clock: rate follows the mids smoothly, so the pattern stays coherent
    float rate = 0.9f + 0.25f * sh[1];
    clock += (rate - clock) * (1.0f - std::exp(-dt / 2.0f));
    tau += double(dt) * clock;

    int N = int(clampf(std::round(countKnob), 4.0f, float(NMAX)));
    count = float(N);
    double cr = 0.0, ci = 0.0;
    for (int i = 0; i < NMAX; i++) {
      double ph = double(TAU) * (K + i) * tau / T;
      angles[i] = amp * float(std::sin(ph));
      float r = K / (K + i);
      lengths[i] = 1.75f * r * r;
      if (i < N) { cr += std::cos(ph); ci += std::sin(ph); }
    }
    align = float(std::sqrt(cr * cr + ci * ci) / N);

    // highs -> sparkle (relative onset); mids -> warmth + slow orbit
    float hi = bands[2];
    hiFast += (hi - hiFast) * (1.0f - std::exp(-dt / 0.06f));
    if (hiAvg <= 0.0f) hiAvg = std::max(hi, 1e-3f);
    hiAvg = std::max(hiAvg + (hi - hiAvg) * (1.0f - std::exp(-dt / 2.0f)), 1e-3f);
    float st = smoothstep(0.08f, 0.5f, (hiFast / hiAvg - 1.0f) * g) * live;
    spark = std::max(st, spark * std::exp(-dt / 0.3f));
    warm += (sh[1] - warm) * (1.0f - std::exp(-dt / 3.0f));
    orbit += ((sh[1] - 0.5f) * 0.5f - orbit) * (1.0f - std::exp(-dt / 5.0f));
  }
};

struct AudioPendulumShader: Shader {
  AudioPendulumSettings *settings;
  AudioPendulumShader(AudioPendulumSettings *settings) : settings(settings), Shader(settings) {};

  PendulumCPU cpu;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/AudioPendulum");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;

    auto source = AudioSourceService::getService()->selectedAudioSource;
    static const std::array<float, 256> silentSpec = {};
    static const std::array<float, AudioAnalysis::kRawHistorySize> silentHist = {};
    const float *spec = source != nullptr ? source->audioAnalysis.renderSpectrum.data() : silentSpec.data();
    const float *hist = source != nullptr ? source->audioAnalysis.renderRawHistory.data() : silentHist.data();
    cpu.update(now, dt, spec, hist, settings->audioSensitivity->value, settings->swing->value,
               settings->kickPunch->value, settings->pendulums->value);

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1fv("audio", spec, 256);
    shader.setUniform1fv("angles", cpu.angles.data(), PendulumCPU::NMAX);
    shader.setUniform1fv("lengths", cpu.lengths.data(), PendulumCPU::NMAX);
    shader.setUniform1f("count", cpu.count);
    shader.setUniform1f("align", cpu.align);
    shader.setUniform1f("kickEnv", cpu.kickEnv);
    shader.setUniform1f("sparkle", cpu.spark);
    shader.setUniform1f("warmth", cpu.warm);
    shader.setUniform1f("orbit", cpu.orbit);
    shader.setUniform1f("live", cpu.live);
    shader.setUniform1f("cameraDrift", settings->cameraDrift->value);
    shader.setUniform1f("keyLight", settings->keyLight->value);
    shader.setUniform1f("depthOfField", settings->depthOfField->value);
    shader.setUniform1f("haze", settings->haze->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", now);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioPendulum; }

  void drawSettings() override {
    CommonViews::H3Title("AudioPendulum");
    CommonViews::ShaderParameter(settings->swing, settings->swingOscillator);
    CommonViews::ShaderParameter(settings->kickPunch, settings->kickPunchOscillator);
    CommonViews::ShaderIntParameter(settings->pendulums);
    CommonViews::ShaderParameter(settings->cameraDrift, settings->cameraDriftOscillator);
    CommonViews::ShaderParameter(settings->keyLight, settings->keyLightOscillator);
    CommonViews::ShaderParameter(settings->depthOfField, settings->depthOfFieldOscillator);
    CommonViews::ShaderParameter(settings->haze, settings->hazeOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
