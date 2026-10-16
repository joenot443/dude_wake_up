//
//  AudioPhasePortraitShader.hpp
//  dude_wake_up
//
//  The sound drawn as light on a phosphor oscilloscope. A beam traces a slowly morphing
//  3D harmonograph whose form follows the spectrum; frame feedback is the phosphor
//  (with a dithered subtractive floor so 8-bit trails always fade to black). A genuine
//  phase portrait rides inside it: one scope-triggered period of the raw waveform,
//  delay-embedded, which locks into a closed loop on sustained notes.
//
//  The geometry and audio analysis run on the CPU (PhaseCPU below, ported 1:1 from the
//  reference implementation used to tune the shader offline) and are passed as uniforms.
//

#ifndef AudioPhasePortraitShader_hpp
#define AudioPhasePortraitShader_hpp

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
#include <vector>

struct AudioPhasePortraitSettings: public ShaderSettings {
  std::shared_ptr<Parameter> persistence;
  std::shared_ptr<WaveformOscillator> persistenceOscillator;

  std::shared_ptr<Parameter> beamIntensity;
  std::shared_ptr<WaveformOscillator> beamIntensityOscillator;

  std::shared_ptr<Parameter> ribbon;
  std::shared_ptr<WaveformOscillator> ribbonOscillator;

  std::shared_ptr<Parameter> portrait;
  std::shared_ptr<WaveformOscillator> portraitOscillator;

  std::shared_ptr<Parameter> kickPunch;
  std::shared_ptr<WaveformOscillator> kickPunchOscillator;

  std::shared_ptr<Parameter> figureSize;
  std::shared_ptr<WaveformOscillator> figureSizeOscillator;

  std::shared_ptr<Parameter> beamSpeed;
  std::shared_ptr<WaveformOscillator> beamSpeedOscillator;

  std::shared_ptr<Parameter> waveJitter;
  std::shared_ptr<WaveformOscillator> waveJitterOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AudioPhasePortraitSettings(std::string shaderId, json j) :
  persistence(std::make_shared<Parameter>("Persistence", 0.8, 0.0, 1.0)),
  persistenceOscillator(std::make_shared<WaveformOscillator>(persistence)),
  beamIntensity(std::make_shared<Parameter>("Beam Intensity", 1.0, 0.0, 3.0)),
  beamIntensityOscillator(std::make_shared<WaveformOscillator>(beamIntensity)),
  ribbon(std::make_shared<Parameter>("Rulings", 1.0, 0.0, 2.0)),
  ribbonOscillator(std::make_shared<WaveformOscillator>(ribbon)),
  portrait(std::make_shared<Parameter>("Phase Portrait", 1.0, 0.0, 2.0)),
  portraitOscillator(std::make_shared<WaveformOscillator>(portrait)),
  kickPunch(std::make_shared<Parameter>("Kick Punch", 1.0, 0.0, 1.0)),
  kickPunchOscillator(std::make_shared<WaveformOscillator>(kickPunch)),
  figureSize(std::make_shared<Parameter>("Figure Size", 1.0, 0.3, 2.0)),
  figureSizeOscillator(std::make_shared<WaveformOscillator>(figureSize)),
  beamSpeed(std::make_shared<Parameter>("Beam Speed", 1.0, 0.1, 3.0)),
  beamSpeedOscillator(std::make_shared<WaveformOscillator>(beamSpeed)),
  waveJitter(std::make_shared<Parameter>("Wave Jitter", 1.0, 0.0, 3.0)),
  waveJitterOscillator(std::make_shared<WaveformOscillator>(waveJitter)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.4, 0.2, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "AudioPhasePortrait") {
    parameters = { persistence, beamIntensity, ribbon, portrait, kickPunch, figureSize, beamSpeed, waveJitter, hueShift, audioSensitivity, brightness };
    oscillators = { persistenceOscillator, beamIntensityOscillator, ribbonOscillator, portraitOscillator, kickPunchOscillator, figureSizeOscillator, beamSpeedOscillator, waveJitterOscillator, hueShiftOscillator, audioSensitivityOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

// CPU-side analysis + geometry for the phase portrait. One per shader instance, so the
// display-flash inverse (flashPrev) always matches what this instance drew last frame.
struct PhasePortraitCPU {
  static constexpr int NB = 28, NF = 96, NE = 96, HIST = 1024;
  static constexpr float TAU = 6.2831853f;

  float shape[4] = {0, 0, 0, 0};
  float kickEnv = 0.0f, flashPrevStored = 0.0f, tonality = 0.0f;
  float period = 112.0f, beamHead = 0.0f;
  float avg[4] = {0, 0, 0, 0};    // slow (8 s) band levels: the figure follows change, not loudness
  float bassAvg = 0.0f;           // 4 s raw bass level for relative kick onsets
  float lastStrobe = -10.0f;

  // outputs
  float flashNow = 0.0f, flashPrev = 0.0f, strobe = 0.0f, portraitGain = 0.0f, figureGain = 1.0f;
  std::array<float, 3 * 2 * (NB + 1)> beamPts{};
  std::array<float, 3 * NF> figurePts{};
  std::array<float, 3 * NE> embedPts{};

  static float clampf(float x, float a, float b) { return std::max(a, std::min(b, x)); }
  static float smoothstep(float a, float b, float x) { float u = clampf((x - a) / (b - a), 0.0f, 1.0f); return u * u * (3.0f - 2.0f * u); }
  static float lerp(float a, float b, float k) { return a + (b - a) * k; }
  static float band(const float *sp, float lo, float hi) {
    float s = 0.0f;
    for (int j = 0; j < 8; j++) s += sp[(int)clampf(int((lo + (hi - lo) * j / 7.0f) * 255.0f), 0, 255)];
    return s / 8.0f;
  }

  // Autocorrelation pitch on the newest 512 history samples, lags up to 400
  // (55 Hz .. 2.7 kHz at the 22.05 kHz downsampled rate). Returns false if aperiodic.
  static bool pitch(const float *hist, float &periodOut, float &peakOut) {
    double x[512], mean = 0.0;
    for (int i = 0; i < 512; i++) { x[i] = hist[HIST - 512 + i]; mean += x[i]; }
    mean /= 512.0;
    double r0 = 0.0;
    for (int i = 0; i < 512; i++) { x[i] -= mean; r0 += x[i] * x[i]; }
    peakOut = 0.0f;
    if (r0 < 1e-6) return false;
    double r[401];
    for (int l = 0; l <= 400; l++) {
      double s = 0.0;
      for (int i = 0; i < 512 - l; i++) s += x[i] * x[i + l];
      r[l] = s / r0 * 512.0 / (512 - l);
    }
    int l0 = -1;
    for (int l = 1; l <= 400; l++) if (r[l] < 0.0) { l0 = l; break; }
    if (l0 < 0 || l0 >= 400) return false;
    int p = l0;
    for (int l = l0; l <= 400; l++) if (r[l] > r[p]) p = l;
    float off = 0.0f;
    if (p > 0 && p < 400) {
      double a = r[p - 1], b = r[p], c = r[p + 1], den = a - 2.0 * b + c;
      off = std::fabs(den) > 1e-9 ? float(0.5 * (a - c) / den) : 0.0f;
    }
    periodOut = p + clampf(off, -0.5f, 0.5f);
    peakOut = float(r[p]);
    return true;
  }

  void update(float t, float dt, const float *sp, const float *hist, float sens,
              float figureSize, float beamSpeed, float waveJitter, float kickPunch) {
    float energy = 0.0f;
    for (int i = 0; i < 256; i += 8) energy += sp[i];
    float live = smoothstep(0.05f, 0.4f, energy);
    float bands[4] = { band(sp, 0.0f, 0.06f), band(sp, 0.06f, 0.20f), band(sp, 0.20f, 0.50f), band(sp, 0.50f, 1.0f) };
    float k = 1.0f - std::exp(-dt / 1.5f);
    float ka = 1.0f - std::exp(-dt / 8.0f);
    if (live > 0.01f && std::max(std::max(avg[0], avg[1]), std::max(avg[2], avg[3])) <= 1e-3f)
      for (int i = 0; i < 4; i++) avg[i] = shape[i] = bands[i];   // seed on first sound
    for (int i = 0; i < 4; i++) {
      shape[i] += (bands[i] - shape[i]) * k;
      avg[i] = std::max(avg[i] + (shape[i] - avg[i]) * ka, 1e-3f);
    }

    // kick: relative onset of the bass over its 4 s average. Bass energy comes from the raw
    // history (RMS of 64-sample block means ~ energy below ~300 Hz) because the spectrum clamps
    // at 1 and saturates on loud masters. Instant attack, 150 ms release, strobe on the rising
    // edge with a 0.45 s refractory period.
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
    float kickPrev = kickEnv;
    float onset = bass / std::max(bassAvg, 1e-3f) - 1.0f;
    float target = smoothstep(0.25f, 0.8f, onset * (0.75f + 0.25f * sens)) * live;
    kickEnv = std::max(target, kickEnv * std::exp(-dt / 0.15f));
    strobe = (kickEnv > 0.5f && kickPrev <= 0.5f && t - lastStrobe > 0.45f) ? 1.0f : 0.0f;
    if (strobe > 0.0f) lastStrobe = t;
    flashPrev = flashPrevStored;
    flashNow = std::min(kickEnv * kickPunch * 0.5f, 1.0f);
    flashPrevStored = flashNow;

    // periodicity -> tonality; period -> tau (quarter period) + scope trigger
    float per, rpk;
    if (pitch(hist, per, rpk)) period = per; else rpk = 0.0f;
    float ton = smoothstep(0.7f, 0.92f, rpk) * live;
    tonality += (ton - tonality) * (1.0f - std::exp(-dt / 0.15f));
    float tau = clampf(period / 4.0f, 2.0f, 100.0f);

    // epochs / form / camera
    // low ratios: legible Lissajous figures
    static const int RATIOS[6][3] = { {1, 2, 3}, {2, 3, 1}, {3, 4, 2}, {1, 3, 2}, {2, 3, 4}, {3, 2, 1} };
    float ep = t / 20.0f, eid = std::floor(ep), ef = ep - eid;
    const int *R0 = RATIOS[((int(eid) * 7 + 3) % 6 + 6) % 6];
    const int *R1 = RATIOS[((int(eid + 1.0f) * 7 + 3) % 6 + 6) % 6];
    float F[3];
    // shape change is a dissolve, not a sweep through open intermediate curves (which persistence
    // stacks into dozens of copies): the figure fades out, swaps ratios while invisible, fades back in
    for (int i = 0; i < 3; i++) F[i] = float(ef >= 0.875f ? R1[i] : R0[i]);
    figureGain = 1.0f - smoothstep(0.80f, 0.86f, ef) * (1.0f - smoothstep(0.89f, 0.94f, ef));
    float dive = ((int(eid) % 3 == 1) ? 1.0f : 0.0f) * smoothstep(0.35f, 0.45f, ef) * (1.0f - smoothstep(0.55f, 0.68f, ef));
    float sh[4];
    if (live > 0.01f) {
      // auto-gained: 0.5 at the band's own 8 s level, toward 0/1 as it falls/rises
      for (int i = 0; i < 4; i++) sh[i] = smoothstep(0.0f, 1.0f, 0.5f + 0.5f * (0.5f + 0.5f * sens) * (shape[i] / avg[i] - 1.0f));
    } else {
      sh[0] = 0.5f + 0.2f * std::sin(t * 0.11f); sh[1] = 0.35f + 0.2f * std::sin(t * 0.07f); sh[2] = 0.25f; sh[3] = 0.15f;
    }
    float F2[3] = { 2 * F[0] + 1, 2 * F[1] + 1, 2 * F[2] };   // fixed overtone: no sudden shape jumps
    float sc = figureSize * 0.66f * (0.92f + 0.12f * sh[0]) * (1.0f + 0.3f * dive);
    float A[3] = { (0.70f + 0.15f * sh[0]) * sc, (0.06f + 0.16f * sh[1]) * sc, (0.015f + 0.06f * (0.6f * sh[2] + 0.4f * sh[3])) * sc };
    const float prec = 0.03f * t;   // slow precession of the y phase: time-based, so the figure stays closed
    float yaw = t * 0.035f + 0.15f * std::sin(t * 0.05f);
    float pit = lerp(0.25f * std::sin(t * 0.07f), 0.6f, dive);
    float cy = std::cos(yaw), sy = std::sin(yaw), cp = std::cos(pit), spi = std::sin(pit);

    auto harm = [&](float s, float out[3]) {
      out[0] = A[0] * std::sin(F[0] * s) + A[2] * std::sin(F2[0] * s + 0.5f);
      out[1] = A[0] * std::sin(F[1] * s + prec + 1.3f) + A[1] * std::sin(F2[1] * s);
      out[2] = A[1] * std::sin(F[2] * s + 2.1f) + A[2] * std::sin(F2[2] * s + 1.1f);
    };
    auto project = [&](const float P[3], float *out) {
      float x = P[0], y = P[1], z = P[2];
      float x2 = cy * x - sy * z, z2 = sy * x + cy * z;
      float y2 = cp * y - spi * z2, z3 = spi * y + cp * z2;
      float persp = 2.2f / (2.2f + 0.8f * z3);
      out[0] = x2 * persp * 1.6f; out[1] = y2 * persp; out[2] = z3;
    };

    // low-passed history (box filter, width period/40) so hats/noise don't make the portrait polygonal
    std::vector<float> H(HIST);
    int wbox = std::max(1, int(std::lround(period / 40.0f)));
    if (wbox > 1) {
      std::vector<double> cs(HIST + 1, 0.0);
      for (int i = 0; i < HIST; i++) cs[i + 1] = cs[i] + hist[i];
      for (int i = 0; i < HIST; i++) {
        int hi = std::min(i + wbox / 2 + 1, HIST), lo = std::max(i - wbox / 2, 0);
        H[i] = float((cs[hi] - cs[lo]) / (hi - lo));
      }
    } else {
      for (int i = 0; i < HIST; i++) H[i] = hist[i];
    }
    auto hw = [&](float u) {
      u = clampf(u, 0.0f, float(HIST - 1)); int i = int(u); float f = u - i; int j = std::min(i + 1, HIST - 1);
      return H[i] * (1.0f - f) + H[j] * f;
    };
    double ss = 0.0;
    for (int i = HIST - 256; i < HIST; i++) ss += H[i] * H[i];
    float rmsN = float(std::sqrt(ss / 256.0));
    auto wj = [&](float u) {
      return rmsN < 1e-4f ? 0.0f : clampf(hw(768.0f + clampf(u, 0.0f, 255.0f)) / rmsN, -2.0f, 2.0f);
    };

    // the beam: this frame's stretch of both pens, with the raw waveform shivering the stroke
    float head0 = beamHead;
    float base = TAU * std::floor(head0 / TAU);   // beam phase wrapped into the figure's [0, TAU) so it rides on the figure
    beamHead += dt * beamSpeed * 3.0f;
    for (int pen = 0; pen < 2; pen++) {
      float amp = pen == 0 ? 1.0f : 1.3f;
      float pts[NB + 1][3];
      for (int j = 0; j <= NB; j++) {
        float h[3]; harm(lerp(head0, beamHead, float(j) / NB) - base - pen * 0.9f, h);
        for (int c = 0; c < 3; c++) h[c] *= amp;
        project(h, pts[j]);
      }
      for (int j = 0; j <= NB; j++) {
        const float *a = pts[std::max(j - 1, 0)], *b = pts[std::min(j + 1, NB)];
        float dx = b[0] - a[0], dy = b[1] - a[1], L = std::hypot(dx, dy) + 1e-6f;
        float s = lerp(head0, beamHead, float(j) / NB);
        float ph = std::fmod(s * 3.0f / TAU, 1.0f); if (ph < 0.0f) ph += 1.0f;
        float d = clampf(0.004f * waveJitter * wj(ph * 255.0f) * 0.5f, -0.004f, 0.004f);
        pts[j][0] += -dy / L * d; pts[j][1] += dx / L * d;
      }
      for (int j = 0; j <= NB; j++)
        for (int c = 0; c < 3; c++) beamPts[(pen * (NB + 1) + j) * 3 + c] = pts[j][c];
    }
    for (int i = 0; i < NF; i++) {
      float h[3]; harm(TAU * i / (NF - 1), h);   // fixed phase: vertices don't slide, so the figure only changes when its shape does
      for (int c = 0; c < 3; c++) h[c] *= 1.06f;
      project(h, &figurePts[i * 3]);
    }

    // scope trigger: first rising zero crossing after 2*tau, then exactly ONE period,
    // delay-embedded (w[u], w[u-tau], w[u-2tau]). A sustained note redraws the same loop.
    float closure = 0.0f;
    bool found = false;
    float z0 = 0.0f;
    int start = int(std::ceil(2.0f * tau)) + 1, stop = int(HIST - 1 - period - 1);
    for (int i = start; i < stop; i++) {
      if (H[i - 1] < 0.0f && H[i] >= 0.0f) { z0 = (i - 1) + (-H[i - 1]) / (H[i] - H[i - 1] + 1e-12f); found = true; break; }
    }
    if (found) {
      double segSS = 0.0;
      for (int n = 0; n < NE; n++) { float v = hw(z0 + period * n / (NE - 1)); segSS += v * v; }
      float rms = float(std::sqrt(segSS / NE)) + 1e-6f;
      float E[NE][3];
      float ext = 0.0f;
      for (int n = 0; n < NE; n++) {
        float u = z0 + period * n / (NE - 1);
        E[n][0] = clampf(hw(u) / rms, -2.5f, 2.5f);
        E[n][1] = clampf(hw(u - tau) / rms, -2.5f, 2.5f);
        E[n][2] = clampf(hw(u - 2.0f * tau) / rms, -2.5f, 2.5f);
        for (int c = 0; c < 3; c++) ext = std::max(ext, std::fabs(E[n][c]));
      }
      ext += 1e-6f;
      float dx = E[0][0] - E[NE - 1][0], dy = E[0][1] - E[NE - 1][1], dz = E[0][2] - E[NE - 1][2];
      // simplicity: arc length over extent (a circle ~6.3, a figure-eight ~9; polyphony scribbles >> 15)
      float arc = 0.0f;
      for (int n = 0; n < NE - 1; n++) {
        float ax = E[n + 1][0] - E[n][0], ay = E[n + 1][1] - E[n][1], az = E[n + 1][2] - E[n][2];
        arc += std::sqrt(ax * ax + ay * ay + az * az);
      }
      closure = smoothstep(0.2f, 0.05f, std::sqrt(dx * dx + dy * dy + dz * dz) / ext) * smoothstep(14.0f, 9.0f, arc / ext);
      for (int n = 0; n < NE; n++) {
        float P[3] = { E[n][0] * 0.30f * sc, E[n][1] * 0.30f * sc, E[n][2] * 0.30f * sc };
        project(P, &embedPts[n * 3]);
      }
    } else {
      embedPts.fill(0.0f);
    }
    portraitGain = tonality * closure;
  }
};

struct AudioPhasePortraitShader: Shader {
  AudioPhasePortraitSettings *settings;
  AudioPhasePortraitShader(AudioPhasePortraitSettings *settings) : settings(settings), Shader(settings) {};

  PhasePortraitCPU cpu;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/AudioPhasePortrait");
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
    cpu.update(now, dt, spec, hist, settings->audioSensitivity->value, settings->figureSize->value,
               settings->beamSpeed->value, settings->waveJitter->value, settings->kickPunch->value);

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1fv("audio", spec, 256);
    shader.setUniform1f("frameDt", dt);
    shader.setUniform3fv("beamPts", cpu.beamPts.data(), int(cpu.beamPts.size() / 3));
    shader.setUniform3fv("figurePts", cpu.figurePts.data(), int(cpu.figurePts.size() / 3));
    shader.setUniform3fv("embedPts", cpu.embedPts.data(), int(cpu.embedPts.size() / 3));
    shader.setUniform1f("portraitGain", cpu.portraitGain);
    shader.setUniform1f("figureGain", cpu.figureGain);
    shader.setUniform1f("kickEnv", cpu.kickEnv);
    shader.setUniform1f("strobe", cpu.strobe);
    shader.setUniform1f("flashNow", cpu.flashNow);
    shader.setUniform1f("flashPrev", cpu.flashPrev);
    shader.setUniform1f("persistence", settings->persistence->value);
    shader.setUniform1f("beamIntensity", settings->beamIntensity->value);
    shader.setUniform1f("ribbon", settings->ribbon->value);
    shader.setUniform1f("portrait", settings->portrait->value);
    shader.setUniform1f("kickPunch", settings->kickPunch->value);
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
  ShaderType type() override { return ShaderTypeAudioPhasePortrait; }

  void drawSettings() override {
    CommonViews::H3Title("AudioPhasePortrait");
    CommonViews::ShaderParameter(settings->persistence, settings->persistenceOscillator);
    CommonViews::ShaderParameter(settings->beamIntensity, settings->beamIntensityOscillator);
    CommonViews::ShaderParameter(settings->ribbon, settings->ribbonOscillator);
    CommonViews::ShaderParameter(settings->portrait, settings->portraitOscillator);
    CommonViews::ShaderParameter(settings->kickPunch, settings->kickPunchOscillator);
    CommonViews::ShaderParameter(settings->figureSize, settings->figureSizeOscillator);
    CommonViews::ShaderParameter(settings->beamSpeed, settings->beamSpeedOscillator);
    CommonViews::ShaderParameter(settings->waveJitter, settings->waveJitterOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
