//
//  AudioRainShader.hpp
//  dude_wake_up
//
//  Rain on the side window of a car driving through a city at night. Outside, the
//  out-of-focus world slides past in parallax: a far skyline drifting slowly, shopfronts
//  and neon signs, oncoming headlights smearing by, roadside lamps whipping past and
//  overhead streetlights sweeping a warm wash across the glass. The drive loops through
//  downtown neon, sodium-lit residential streets, a sparse highway and a tunnel, stopping
//  at red lights along the way. On the glass, beads and drops act as tiny lenses; airflow
//  pushes them back along the window at speed, and they slide straight down when parked.
//
//  Everything that moves is integrated on the CPU (RainCPU below, ported 1:1 from the
//  reference used to tune the shader offline on real tracks), so speed and knob changes
//  never jump positions.
//

#ifndef AudioRainShader_hpp
#define AudioRainShader_hpp

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
#include <cstdint>

struct AudioRainSettings: public ShaderSettings {
  std::shared_ptr<Parameter> driveSpeed;
  std::shared_ptr<WaveformOscillator> driveSpeedOscillator;

  std::shared_ptr<Parameter> direction;

  std::shared_ptr<Parameter> scene;

  std::shared_ptr<Parameter> routePace;
  std::shared_ptr<WaveformOscillator> routePaceOscillator;

  std::shared_ptr<Parameter> redLights;
  std::shared_ptr<WaveformOscillator> redLightsOscillator;

  std::shared_ptr<Parameter> parallaxDepth;
  std::shared_ptr<WaveformOscillator> parallaxDepthOscillator;

  std::shared_ptr<Parameter> traffic;
  std::shared_ptr<WaveformOscillator> trafficOscillator;

  std::shared_ptr<Parameter> streetlights;
  std::shared_ptr<WaveformOscillator> streetlightsOscillator;

  std::shared_ptr<Parameter> headlightSweeps;
  std::shared_ptr<WaveformOscillator> headlightSweepsOscillator;

  std::shared_ptr<Parameter> lightning;
  std::shared_ptr<WaveformOscillator> lightningOscillator;

  std::shared_ptr<Parameter> rainAmount;
  std::shared_ptr<WaveformOscillator> rainAmountOscillator;

  std::shared_ptr<Parameter> windStreak;
  std::shared_ptr<WaveformOscillator> windStreakOscillator;

  std::shared_ptr<Parameter> dropSize;
  std::shared_ptr<WaveformOscillator> dropSizeOscillator;

  std::shared_ptr<Parameter> trailLength;
  std::shared_ptr<WaveformOscillator> trailLengthOscillator;

  std::shared_ptr<Parameter> windowFog;
  std::shared_ptr<WaveformOscillator> windowFogOscillator;

  std::shared_ptr<Parameter> fogWipe;
  std::shared_ptr<WaveformOscillator> fogWipeOscillator;

  std::shared_ptr<Parameter> focus;
  std::shared_ptr<WaveformOscillator> focusOscillator;

  std::shared_ptr<Parameter> bokehSize;
  std::shared_ptr<WaveformOscillator> bokehSizeOscillator;

  std::shared_ptr<Parameter> apertureBlades;

  std::shared_ptr<Parameter> cityGlow;
  std::shared_ptr<WaveformOscillator> cityGlowOscillator;

  std::shared_ptr<Parameter> neonMix;
  std::shared_ptr<WaveformOscillator> neonMixOscillator;

  std::shared_ptr<Parameter> interiorGlow;
  std::shared_ptr<WaveformOscillator> interiorGlowOscillator;

  std::shared_ptr<Parameter> colorTemperature;
  std::shared_ptr<WaveformOscillator> colorTemperatureOscillator;

  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<WaveformOscillator> saturationOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> vignette;
  std::shared_ptr<WaveformOscillator> vignetteOscillator;

  std::shared_ptr<Parameter> filmGrain;
  std::shared_ptr<WaveformOscillator> filmGrainOscillator;

  std::shared_ptr<Parameter> kickPunch;
  std::shared_ptr<WaveformOscillator> kickPunchOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  AudioRainSettings(std::string shaderId, json j) :
  driveSpeed(std::make_shared<Parameter>("Drive Speed", 1.0, 0.0, 2.0)),
  driveSpeedOscillator(std::make_shared<WaveformOscillator>(driveSpeed)),
  direction(std::make_shared<Parameter>("Direction", 0.0, 0.0, 1.0, ParameterType_Bool)),
  scene(std::make_shared<Parameter>("Scene", 0.0, 0.0, 4.0, ParameterType_Int)),
  routePace(std::make_shared<Parameter>("Route Pace", 1.0, 0.25, 3.0)),
  routePaceOscillator(std::make_shared<WaveformOscillator>(routePace)),
  redLights(std::make_shared<Parameter>("Red Lights", 1.0, 0.0, 1.0)),
  redLightsOscillator(std::make_shared<WaveformOscillator>(redLights)),
  parallaxDepth(std::make_shared<Parameter>("Parallax Depth", 1.0, 0.0, 2.0)),
  parallaxDepthOscillator(std::make_shared<WaveformOscillator>(parallaxDepth)),
  traffic(std::make_shared<Parameter>("Traffic", 1.0, 0.0, 2.0)),
  trafficOscillator(std::make_shared<WaveformOscillator>(traffic)),
  streetlights(std::make_shared<Parameter>("Streetlights", 1.0, 0.0, 2.0)),
  streetlightsOscillator(std::make_shared<WaveformOscillator>(streetlights)),
  headlightSweeps(std::make_shared<Parameter>("Headlight Sweeps", 1.0, 0.0, 2.0)),
  headlightSweepsOscillator(std::make_shared<WaveformOscillator>(headlightSweeps)),
  lightning(std::make_shared<Parameter>("Lightning", 0.3, 0.0, 1.0)),
  lightningOscillator(std::make_shared<WaveformOscillator>(lightning)),
  rainAmount(std::make_shared<Parameter>("Rain Amount", 1.0, 0.0, 2.0)),
  rainAmountOscillator(std::make_shared<WaveformOscillator>(rainAmount)),
  windStreak(std::make_shared<Parameter>("Wind Streak", 1.0, 0.0, 2.0)),
  windStreakOscillator(std::make_shared<WaveformOscillator>(windStreak)),
  dropSize(std::make_shared<Parameter>("Drop Size", 1.0, 0.5, 2.0)),
  dropSizeOscillator(std::make_shared<WaveformOscillator>(dropSize)),
  trailLength(std::make_shared<Parameter>("Trail Length", 1.0, 0.0, 2.0)),
  trailLengthOscillator(std::make_shared<WaveformOscillator>(trailLength)),
  windowFog(std::make_shared<Parameter>("Window Fog", 0.4, 0.0, 1.0)),
  windowFogOscillator(std::make_shared<WaveformOscillator>(windowFog)),
  fogWipe(std::make_shared<Parameter>("Fog Wipe", 0.0, 0.0, 1.0)),
  fogWipeOscillator(std::make_shared<WaveformOscillator>(fogWipe)),
  focus(std::make_shared<Parameter>("Focus", 0.4, 0.0, 1.0)),
  focusOscillator(std::make_shared<WaveformOscillator>(focus)),
  bokehSize(std::make_shared<Parameter>("Bokeh Size", 1.0, 0.4, 1.6)),
  bokehSizeOscillator(std::make_shared<WaveformOscillator>(bokehSize)),
  apertureBlades(std::make_shared<Parameter>("Aperture Blades", 4.0, 4.0, 8.0, ParameterType_Int)),
  cityGlow(std::make_shared<Parameter>("City Glow", 1.0, 0.0, 2.0)),
  cityGlowOscillator(std::make_shared<WaveformOscillator>(cityGlow)),
  neonMix(std::make_shared<Parameter>("Neon Mix", 1.0, 0.0, 2.0)),
  neonMixOscillator(std::make_shared<WaveformOscillator>(neonMix)),
  interiorGlow(std::make_shared<Parameter>("Interior Glow", 0.3, 0.0, 1.0)),
  interiorGlowOscillator(std::make_shared<WaveformOscillator>(interiorGlow)),
  colorTemperature(std::make_shared<Parameter>("Color Temperature", 0.0, -1.0, 1.0)),
  colorTemperatureOscillator(std::make_shared<WaveformOscillator>(colorTemperature)),
  saturation(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 2.0)),
  saturationOscillator(std::make_shared<WaveformOscillator>(saturation)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  vignette(std::make_shared<Parameter>("Vignette", 0.45, 0.0, 1.0)),
  vignetteOscillator(std::make_shared<WaveformOscillator>(vignette)),
  filmGrain(std::make_shared<Parameter>("Film Grain", 0.0, 0.0, 1.0)),
  filmGrainOscillator(std::make_shared<WaveformOscillator>(filmGrain)),
  kickPunch(std::make_shared<Parameter>("Kick Punch", 0.8, 0.0, 1.0)),
  kickPunchOscillator(std::make_shared<WaveformOscillator>(kickPunch)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 1.0, 0.0, 4.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.2, 4.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "AudioRain") {
    parameters = { driveSpeed, direction, scene, routePace, redLights, parallaxDepth, traffic, streetlights, headlightSweeps, lightning, rainAmount, windStreak, dropSize, trailLength, windowFog, fogWipe, focus, bokehSize, apertureBlades, cityGlow, neonMix, interiorGlow, colorTemperature, saturation, hueShift, vignette, filmGrain, kickPunch, audioSensitivity, brightness };
    oscillators = { driveSpeedOscillator, routePaceOscillator, redLightsOscillator, parallaxDepthOscillator, trafficOscillator, streetlightsOscillator, headlightSweepsOscillator, lightningOscillator, rainAmountOscillator, windStreakOscillator, dropSizeOscillator, trailLengthOscillator, windowFogOscillator, fogWipeOscillator, focusOscillator, bokehSizeOscillator, cityGlowOscillator, neonMixOscillator, interiorGlowOscillator, colorTemperatureOscillator, saturationOscillator, hueShiftOscillator, vignetteOscillator, filmGrainOscillator, kickPunchOscillator, audioSensitivityOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

// CPU side: audio analysis, the drive (route, scene, speed, stops, events) and every integrated
// position, so knob changes change rates or crossfade - they never jump.
// Route (Auto scene; advances only while driving, at Route Pace): a 240 s loop - downtown 0-60
// (red light 40-50), residential 60-120 (red light 100-109), highway 120-172, tunnel 172-196,
// downtown 196-240. Scene 1-4 locks an area; the area weights crossfade over ~2 s.
struct RainCPU {
  static constexpr int HIST = 1024;
  static constexpr float ROUTE = 240.0f, WRAP = 4096.0f;
  float bassAvg = 0.0f, kickEnv = 0.0f;
  float mid = 0.0f, midAvg = 0.0f, high = 0.0f, highAvg = 0.0f;
  float bassLvl = 0.0f, bassSlow = 0.0f;
  float rainDrive = 0.3f, highLvl = 0.0f, live = 0.0f;
  bool seeded = false;
  float route = 0.0f, speed = 0.0f, wind = 0.0f;
  float w[4] = {1.0f, 0.0f, 0.0f, 0.0f};
  float offFar = 0.0f, offMid = 0.0f, offNear = 0.0f, offCar = 0.0f, dropClock = 0.0f;
  float sweepT = 10.0f, sweepAmpLatched = 0.0f, kickPrev = 0.0f;
  double clock = 0.0, lastSweep = -10.0;   // event clock in double: stays exact over hours of runtime
  int flashN = 0;
  float flashT = 10.0f, flashAmp = 0.0f;
  double nextFlash = 6.0, lastFlash = -10.0;
  // outputs
  float wDown = 1.0f, wRes = 0.0f, wHwy = 0.0f, wTun = 0.0f, sigRed = 0.0f, sigGreen = 0.0f;
  float sweepX = -1.4f, sweepAmp = 0.0f, flash = 0.0f, flashX = 0.0f, blades = 4.0f;

  static float clampf(float x, float a, float b) { return std::max(a, std::min(b, x)); }
  static float smoothstep(float a, float b, float x) { float u = clampf((x - a) / (b - a), 0.0f, 1.0f); return u * u * (3.0f - 2.0f * u); }
  static float win(float x, float a, float b, float f) { return smoothstep(a - f, a + f, x) * (1.0f - smoothstep(b - f, b + f, x)); }
  static float band(const float *sp, float lo, float hi) {
    float s = 0.0f;
    for (int j = 0; j < 8; j++) s += sp[(int)clampf(int((lo + (hi - lo) * j / 7.0f) * 255.0f), 0, 255)];
    return s / 8.0f;
  }
  // integer hash -> [0,1) (identical to the Python reference)
  static float hash01(uint32_t n) {
    uint32_t x = n * 2654435761u;                    // golden-ratio pre-mix (small n otherwise cluster)
    x = x * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    x = (x >> 22u) ^ x;
    return float(double(x) / 4294967296.0);
  }
  static void routeWeights(float r, float out[4]) {
    float tun = win(r, 172.0f, 196.0f, 1.2f);
    float res = win(r, 60.0f, 120.0f, 4.0f);
    float hwy = win(r, 120.0f, 172.0f, 4.0f) * (1.0f - tun);
    float down = std::max(0.0f, 1.0f - res - hwy - tun);
    float s = down + res + hwy + tun;
    out[0] = down / s; out[1] = res / s; out[2] = hwy / s; out[3] = tun / s;
  }
  // one lightning stroke: ramps up over ~7 frames, decays over ~0.25 s (never a one-frame strobe)
  static float flashEnv(float s) {
    if (s < 0.0f) return 0.0f;
    if (s < 0.12f) return smoothstep(0.0f, 0.12f, s);
    return std::exp(-(s - 0.12f) / 0.25f);
  }

  void update(float dt, const float *sp, const float *hist, float sens, float driveSpeed, float depth, float airflow,
              int scene, float routePace, float redLights, float lightning, float sweeps, int bladesKnob) {
    clock += dt;
    float energy = 0.0f;
    for (int i = 0; i < 256; i += 8) energy += sp[i];
    float liveT = smoothstep(0.05f, 0.4f, energy);
    live += (liveT - live) * (1.0f - std::exp(-dt / 0.5f));

    double bm2 = 0.0;
    for (int b = 0; b < HIST; b += 64) {
      double m = 0.0;
      for (int i = 0; i < 64; i++) m += hist[b + i];
      m /= 64.0;
      bm2 += m * m;
    }
    float bass = float(std::sqrt(bm2 / (HIST / 64)));
    float midB = band(sp, 0.06f, 0.25f), highB = band(sp, 0.40f, 1.0f);
    if (!seeded && liveT > 0.01f) {   // seed: no false onsets while the averages warm up
      bassAvg = bass; mid = midAvg = midB; high = highAvg = highB;
      bassLvl = bassSlow = bass;
      seeded = true;
    }
    // kick: relative bass onset (raw-waveform energy; the spectrum clamps on loud masters)
    bassAvg += (bass - bassAvg) * (1.0f - std::exp(-dt / 4.0f));
    float onset = bass / std::max(bassAvg, 1e-3f) - 1.0f;
    float target = smoothstep(0.3f, 0.9f, onset * (0.75f + 0.25f * sens)) * liveT;
    kickEnv = std::max(target, kickEnv * std::exp(-dt / 0.18f));
    // bass level against its 12 s average: a gentle speed surge (accelerate / coast)
    bassLvl += (bass - bassLvl) * (1.0f - std::exp(-dt / 1.5f));
    bassSlow = std::max(bassSlow + (bassLvl - bassSlow) * (1.0f - std::exp(-dt / 12.0f)), 1e-3f);
    float surge = clampf((bassLvl / bassSlow - 1.0f) * (0.5f + 0.5f * sens), -1.0f, 1.0f) * live;
    // mids / highs auto-gained against their own 8 s levels
    mid += (midB - mid) * (1.0f - std::exp(-dt / 0.8f));
    midAvg = std::max(midAvg + (mid - midAvg) * (1.0f - std::exp(-dt / 8.0f)), 1e-3f);
    high += (highB - high) * (1.0f - std::exp(-dt / 0.3f));
    highAvg = std::max(highAvg + (high - highAvg) * (1.0f - std::exp(-dt / 8.0f)), 1e-3f);
    float g = 0.5f + 0.5f * sens;
    float midN = smoothstep(0.0f, 1.0f, 0.5f + 0.5f * g * (mid / midAvg - 1.0f));
    float highN = smoothstep(0.0f, 1.0f, 0.5f + 0.5f * g * (high / highAvg - 1.0f));
    float rainT = 0.3f + (0.25f + 0.6f * midN - 0.3f) * live;
    rainDrive += (rainT - rainDrive) * (1.0f - std::exp(-dt / 2.0f));
    highLvl += (highN * live - highLvl) * (1.0f - std::exp(-dt / 0.25f));

    // route + scene (a locked scene crossfades over ~2 s; Auto follows the route)
    float driving = smoothstep(0.02f, 0.2f, driveSpeed);
    route = std::fmod(route + dt * driving * routePace, ROUTE);
    int sc = std::max(0, std::min(4, scene));
    float tw[4];
    if (sc == 0) routeWeights(route, tw);
    else for (int i = 0; i < 4; i++) tw[i] = (i == sc - 1) ? 1.0f : 0.0f;
    float kw = 1.0f - std::exp(-dt / 0.7f), ws = 0.0f;
    for (int i = 0; i < 4; i++) { w[i] += (tw[i] - w[i]) * kw; ws += w[i]; }
    for (int i = 0; i < 4; i++) w[i] /= ws;
    wDown = w[0]; wRes = w[1]; wHwy = w[2]; wTun = w[3];
    float stop = 0.0f; sigRed = 0.0f; sigGreen = 0.0f;
    const float stops[2][2] = { {40.0f, 50.0f}, {100.0f, 109.0f} };
    for (int i = 0; i < 2; i++) {
      float a = stops[i][0], b = stops[i][1];
      stop = std::max(stop, win(route, a, b - 1.5f, 0.8f));
      sigRed = std::max(sigRed, win(route, a - 3.0f, b - 2.0f, 0.6f));
      sigGreen = std::max(sigGreen, win(route, b - 2.0f, b + 2.5f, 0.5f));
    }
    float streets = (wDown + wRes) * redLights;   // red lights only on city streets, scaled by the knob
    stop *= streets; sigRed *= streets; sigGreen *= streets;
    float areaSpeed = 0.55f * wDown + 0.75f * wRes + 1.0f * wHwy + 0.95f * wTun;
    float tgt = driveSpeed * areaSpeed * (1.0f - stop) * (1.0f + 0.18f * surge);
    float tau = tgt > speed ? 2.6f : 1.8f;            // pull away a little slower than braking
    speed += (tgt - speed) * (1.0f - std::exp(-dt / tau));
    float windT = clampf(speed * airflow, 0.0f, 2.0f);
    wind += (windT - wind) * (1.0f - std::exp(-dt / 1.2f));
    // parallax: each layer's offset is integrated (rate ~ 1/depth)
    float v = speed;
    offFar = std::fmod(offFar + dt * v * 0.03f, WRAP);
    offMid = std::fmod(offMid + dt * v * 0.38f * (0.6f + 0.4f * depth), WRAP);
    offNear = std::fmod(offNear + dt * v * 1.7f * depth, WRAP);
    offCar = std::fmod(offCar + dt * (v * 0.9f + 0.12f), WRAP);
    dropClock = std::fmod(dropClock + dt * (0.22f + 0.25f * wind), WRAP);

    // kick -> an overtaking car's headlights sweep along the glass (Headlight Sweeps: 0 = none)
    bool rising = kickEnv > 0.6f && kickPrev <= 0.6f;
    bool strong = kickEnv > 0.9f && kickPrev <= 0.9f;
    kickPrev = kickEnv;
    sweepT += dt;
    if (sweeps > 0.01f && rising && clock - lastSweep > 1.6f / std::max(sweeps, 0.3f) && sweepT > 1.0f) {
      sweepT = 0.0f; lastSweep = clock;
      sweepAmpLatched = clampf(0.6f + 0.4f * kickEnv, 0.0f, 1.0f) * std::min(sweeps, 1.5f);
    }
    float ph = clampf(sweepT / 0.9f, 0.0f, 1.0f);
    sweepX = -1.4f + 2.8f * ph;
    sweepAmp = sweepT < 0.9f ? sweepAmpLatched * std::sin(3.14159265f * ph) : 0.0f;

    // lightning: rare distant sheet flashes on a hashed schedule; strong kicks can trigger one when high
    flashT += dt;
    if (lightning < 0.01f) {
      nextFlash = clock + 4.0f;
    } else {
      bool kickTrig = lightning > 0.6f && strong && clock - lastFlash > 8.0f;
      if (clock >= nextFlash || kickTrig) {
        flashN++; flashT = 0.0f; lastFlash = clock;
        flashX = -0.8f + 1.6f * hash01(uint32_t(flashN * 2 + 1));
        flashAmp = 0.55f + 0.45f * hash01(uint32_t(flashN * 2 + 2));
        nextFlash = clock + double((6.0f + 28.0f * hash01(uint32_t(flashN * 2 + 3))) / std::max(lightning, 0.05f));
      }
    }
    flash = flashAmp * (flashEnv(flashT) + 0.6f * flashEnv(flashT - 0.35f)) * (0.5f + 0.5f * std::min(lightning, 1.0f));
    // Aperture Blades eases between shapes instead of snapping
    float bt = clampf(float(bladesKnob), 4.0f, 8.0f);
    blades += (bt - blades) * (1.0f - std::exp(-dt / 0.25f));
  }
};

struct AudioRainShader: Shader {
  AudioRainSettings *settings;
  AudioRainShader(AudioRainSettings *settings) : settings(settings), Shader(settings) {};

  RainCPU cpu;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/AudioRain");
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
    auto *st = settings;
    cpu.update(dt, spec, hist, st->audioSensitivity->value, st->driveSpeed->value, st->parallaxDepth->value,
               st->windStreak->value, st->scene->intValue, st->routePace->value, st->redLights->value,
               st->lightning->value, st->headlightSweeps->value, st->apertureBlades->intValue);

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1fv("audio", spec, 256);
    // CPU-integrated / analysed
    shader.setUniform1f("kickEnv", cpu.kickEnv);
    shader.setUniform1f("rainDrive", cpu.rainDrive);
    shader.setUniform1f("highLvl", cpu.highLvl);
    shader.setUniform1f("live", cpu.live);
    shader.setUniform1f("speed", cpu.speed);
    shader.setUniform1f("wind", cpu.wind);
    shader.setUniform1f("dropClock", cpu.dropClock);
    shader.setUniform1f("offFar", cpu.offFar);
    shader.setUniform1f("offMid", cpu.offMid);
    shader.setUniform1f("offNear", cpu.offNear);
    shader.setUniform1f("offCar", cpu.offCar);
    shader.setUniform1f("wDown", cpu.wDown);
    shader.setUniform1f("wRes", cpu.wRes);
    shader.setUniform1f("wHwy", cpu.wHwy);
    shader.setUniform1f("wTun", cpu.wTun);
    shader.setUniform1f("sigRed", cpu.sigRed);
    shader.setUniform1f("sigGreen", cpu.sigGreen);
    shader.setUniform1f("sweepX", cpu.sweepX);
    shader.setUniform1f("sweepAmp", cpu.sweepAmp);
    shader.setUniform1f("flash", cpu.flash);
    shader.setUniform1f("flashX", cpu.flashX);
    // knobs
    shader.setUniform1f("rainAmount", st->rainAmount->value);
    shader.setUniform1f("dropSize", st->dropSize->value);
    shader.setUniform1f("trailLength", st->trailLength->value);
    shader.setUniform1f("windowFog", st->windowFog->value);
    shader.setUniform1f("fogWipe", st->fogWipe->value);
    shader.setUniform1f("focus", st->focus->value);
    shader.setUniform1f("bokehSize", st->bokehSize->value);
    shader.setUniform1f("blades", cpu.blades);
    shader.setUniform1f("cityGlow", st->cityGlow->value);
    shader.setUniform1f("neonMix", st->neonMix->value);
    shader.setUniform1f("traffic", st->traffic->value);
    shader.setUniform1f("streetlights", st->streetlights->value);
    shader.setUniform1f("interiorGlow", st->interiorGlow->value);
    shader.setUniform1f("colorTemp", st->colorTemperature->value);
    shader.setUniform1f("saturation", st->saturation->value);
    shader.setUniform1f("hueShift", st->hueShift->value);
    shader.setUniform1f("vignette", st->vignette->value);
    shader.setUniform1f("grain", st->filmGrain->value);
    shader.setUniform1f("direction", st->direction->boolValue ? 1.0f : 0.0f);
    shader.setUniform1f("kickPunch", st->kickPunch->value);
    shader.setUniform1f("brightness", st->brightness->value);
    shader.setUniform1f("time", now);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioRain; }

  void drawSettings() override {
    CommonViews::H3Title("AudioRain");
    CommonViews::ShaderParameter(settings->driveSpeed, settings->driveSpeedOscillator);
    CommonViews::ShaderCheckbox(settings->direction);
    CommonViews::ShaderIntParameter(settings->scene);
    CommonViews::ShaderParameter(settings->routePace, settings->routePaceOscillator);
    CommonViews::ShaderParameter(settings->redLights, settings->redLightsOscillator);
    CommonViews::ShaderParameter(settings->parallaxDepth, settings->parallaxDepthOscillator);
    CommonViews::ShaderParameter(settings->traffic, settings->trafficOscillator);
    CommonViews::ShaderParameter(settings->streetlights, settings->streetlightsOscillator);
    CommonViews::ShaderParameter(settings->headlightSweeps, settings->headlightSweepsOscillator);
    CommonViews::ShaderParameter(settings->lightning, settings->lightningOscillator);
    CommonViews::ShaderParameter(settings->rainAmount, settings->rainAmountOscillator);
    CommonViews::ShaderParameter(settings->windStreak, settings->windStreakOscillator);
    CommonViews::ShaderParameter(settings->dropSize, settings->dropSizeOscillator);
    CommonViews::ShaderParameter(settings->trailLength, settings->trailLengthOscillator);
    CommonViews::ShaderParameter(settings->windowFog, settings->windowFogOscillator);
    CommonViews::ShaderParameter(settings->fogWipe, settings->fogWipeOscillator);
    CommonViews::ShaderParameter(settings->focus, settings->focusOscillator);
    CommonViews::ShaderParameter(settings->bokehSize, settings->bokehSizeOscillator);
    CommonViews::ShaderIntParameter(settings->apertureBlades);
    CommonViews::ShaderParameter(settings->cityGlow, settings->cityGlowOscillator);
    CommonViews::ShaderParameter(settings->neonMix, settings->neonMixOscillator);
    CommonViews::ShaderParameter(settings->interiorGlow, settings->interiorGlowOscillator);
    CommonViews::ShaderParameter(settings->colorTemperature, settings->colorTemperatureOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->vignette, settings->vignetteOscillator);
    CommonViews::ShaderParameter(settings->filmGrain, settings->filmGrainOscillator);
    CommonViews::ShaderParameter(settings->kickPunch, settings->kickPunchOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif
