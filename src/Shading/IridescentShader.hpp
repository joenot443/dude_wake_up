//
//  IridescentShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef IridescentShader_hpp
#define IridescentShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct IridescentSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> angle;
  std::shared_ptr<WaveformOscillator> angleOscillator;

  std::shared_ptr<Parameter> layerCount;

  std::shared_ptr<Parameter> frequency;
  std::shared_ptr<WaveformOscillator> frequencyOscillator;

  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<WaveformOscillator> amplitudeOscillator;

  std::shared_ptr<Parameter> amplitudeWobble;
  std::shared_ptr<WaveformOscillator> amplitudeWobbleOscillator;

  std::shared_ptr<Parameter> thickness;
  std::shared_ptr<WaveformOscillator> thicknessOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> colorSpread;
  std::shared_ptr<WaveformOscillator> colorSpreadOscillator;

  std::shared_ptr<Parameter> layerHueOffset;
  std::shared_ptr<WaveformOscillator> layerHueOffsetOscillator;

  std::shared_ptr<Parameter> colorCycleSpeed;
  std::shared_ptr<WaveformOscillator> colorCycleSpeedOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> waveShape;
  std::shared_ptr<WaveformOscillator> waveShapeOscillator;

  std::shared_ptr<Parameter> harmonics;
  std::shared_ptr<WaveformOscillator> harmonicsOscillator;

  std::shared_ptr<Parameter> warpAmount;
  std::shared_ptr<WaveformOscillator> warpAmountOscillator;

  std::shared_ptr<Parameter> warpFreq;
  std::shared_ptr<WaveformOscillator> warpFreqOscillator;

  std::shared_ptr<Parameter> skew;
  std::shared_ptr<WaveformOscillator> skewOscillator;

  std::shared_ptr<Parameter> turbulence;
  std::shared_ptr<WaveformOscillator> turbulenceOscillator;

  std::shared_ptr<Parameter> detune;
  std::shared_ptr<WaveformOscillator> detuneOscillator;

  IridescentSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  angle(std::make_shared<Parameter>("Angle", 0.0, -180.0, 180.0)),
  angleOscillator(std::make_shared<WaveformOscillator>(angle)),
  layerCount(std::make_shared<Parameter>("Layer Count", 10.0, 1.0, 40.0, ParameterType_Int)),
  frequency(std::make_shared<Parameter>("Frequency", 2.0, 0.25, 12.0)),
  frequencyOscillator(std::make_shared<WaveformOscillator>(frequency)),
  amplitude(std::make_shared<Parameter>("Amplitude", 0.25, 0.0, 1.0)),
  amplitudeOscillator(std::make_shared<WaveformOscillator>(amplitude)),
  amplitudeWobble(std::make_shared<Parameter>("Amplitude Wobble", 0.25, 0.0, 1.0)),
  amplitudeWobbleOscillator(std::make_shared<WaveformOscillator>(amplitudeWobble)),
  thickness(std::make_shared<Parameter>("Thickness", 0.01, 0.002, 0.15)),
  thicknessOscillator(std::make_shared<WaveformOscillator>(thickness)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  colorSpread(std::make_shared<Parameter>("Color Spread", 0.5, 0.0, 3.0)),
  colorSpreadOscillator(std::make_shared<WaveformOscillator>(colorSpread)),
  layerHueOffset(std::make_shared<Parameter>("Layer Hue Offset", 1.0, 0.0, 4.0)),
  layerHueOffsetOscillator(std::make_shared<WaveformOscillator>(layerHueOffset)),
  colorCycleSpeed(std::make_shared<Parameter>("Color Cycle Speed", 0.5, 0.0, 4.0)),
  colorCycleSpeedOscillator(std::make_shared<WaveformOscillator>(colorCycleSpeed)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.0, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  waveShape(std::make_shared<Parameter>("Waveform Morph", 0.0, 0.0, 3.0)),
  waveShapeOscillator(std::make_shared<WaveformOscillator>(waveShape)),
  harmonics(std::make_shared<Parameter>("Harmonics", 0.0, 0.0, 1.0)),
  harmonicsOscillator(std::make_shared<WaveformOscillator>(harmonics)),
  warpAmount(std::make_shared<Parameter>("Warp (FM)", 0.0, 0.0, 3.0)),
  warpAmountOscillator(std::make_shared<WaveformOscillator>(warpAmount)),
  warpFreq(std::make_shared<Parameter>("Warp Frequency", 3.0, 0.25, 16.0)),
  warpFreqOscillator(std::make_shared<WaveformOscillator>(warpFreq)),
  skew(std::make_shared<Parameter>("Skew", 0.0, -1.0, 1.0)),
  skewOscillator(std::make_shared<WaveformOscillator>(skew)),
  turbulence(std::make_shared<Parameter>("Turbulence", 0.0, 0.0, 1.0)),
  turbulenceOscillator(std::make_shared<WaveformOscillator>(turbulence)),
  detune(std::make_shared<Parameter>("Detune", 0.0, 0.0, 1.0)),
  detuneOscillator(std::make_shared<WaveformOscillator>(detune)),
  ShaderSettings(shaderId, j, "Iridescent") {
    parameters = { speed, angle, layerCount, frequency, amplitude, amplitudeWobble,
                   thickness, hueShift, colorSpread, layerHueOffset, colorCycleSpeed, brightness,
                   waveShape, harmonics, warpAmount, warpFreq, skew, turbulence, detune };
    oscillators = { speedOscillator, angleOscillator, frequencyOscillator, amplitudeOscillator,
                    amplitudeWobbleOscillator, thicknessOscillator, hueShiftOscillator,
                    colorSpreadOscillator, layerHueOffsetOscillator, colorCycleSpeedOscillator,
                    brightnessOscillator, waveShapeOscillator, harmonicsOscillator,
                    warpAmountOscillator, warpFreqOscillator, skewOscillator,
                    turbulenceOscillator, detuneOscillator };
    load(j);
    registerParameters();
  };
};

struct IridescentShader: Shader {
  IridescentSettings *settings;
  IridescentShader(IridescentSettings *settings) : settings(settings), Shader(settings) {};

  // Speed is integrated into an accumulated phase rather than multiplying
  // absolute time, so changing Speed alters the rate going forward instead of
  // instantly remapping (warping) the whole scene. See shade().
  float speedPhase = 0.0f;
  float lastTimeSample = 0.0f;
  bool timeSampled = false;

  void setup() override {
    shader.load("shaders/Iridescent");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    // Integrate Speed into an accumulated phase: phase += dt * speed. This makes
    // Speed a rate of change so adjusting it (or audio-driving it) eases the
    // motion instead of warping the entire field. Guards skip the first frame
    // and any large time gap (pause / scrub) so the phase never lurches.
    float now = TimeService::getService()->timeParam->value;
    float dt = timeSampled ? (now - lastTimeSample) : 0.0f;
    if (dt < 0.0f || dt > 0.25f) dt = 0.0f;
    lastTimeSample = now;
    timeSampled = true;
    speedPhase += dt * settings->speed->value;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("angle", settings->angle->value * M_PI / 180.0);
    shader.setUniform1f("layerCount", settings->layerCount->value);
    shader.setUniform1f("frequency", settings->frequency->value);
    shader.setUniform1f("amplitude", settings->amplitude->value);
    shader.setUniform1f("amplitudeWobble", settings->amplitudeWobble->value);
    shader.setUniform1f("thickness", settings->thickness->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("colorSpread", settings->colorSpread->value);
    shader.setUniform1f("layerHueOffset", settings->layerHueOffset->value);
    shader.setUniform1f("colorCycleSpeed", settings->colorCycleSpeed->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("waveShape", settings->waveShape->value);
    shader.setUniform1f("harmonics", settings->harmonics->value);
    shader.setUniform1f("warpAmount", settings->warpAmount->value);
    shader.setUniform1f("warpFreq", settings->warpFreq->value);
    shader.setUniform1f("skew", settings->skew->value);
    shader.setUniform1f("turbulence", settings->turbulence->value);
    shader.setUniform1f("detune", settings->detune->value);
    // Speed-scaled accumulated phase (not raw time * speed) — see top of shade().
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
    return ShaderTypeIridescent;
  }

  void drawSettings() override {
    CommonViews::H3Title("Iridescent");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->angle, settings->angleOscillator);
    CommonViews::ShaderIntParameter(settings->layerCount);
    CommonViews::ShaderParameter(settings->frequency, settings->frequencyOscillator);
    CommonViews::ShaderParameter(settings->amplitude, settings->amplitudeOscillator);
    CommonViews::ShaderParameter(settings->amplitudeWobble, settings->amplitudeWobbleOscillator);
    CommonViews::ShaderParameter(settings->thickness, settings->thicknessOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->colorSpread, settings->colorSpreadOscillator);
    CommonViews::ShaderParameter(settings->layerHueOffset, settings->layerHueOffsetOscillator);
    CommonViews::ShaderParameter(settings->colorCycleSpeed, settings->colorCycleSpeedOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);

    CommonViews::H4Title("Waveform Shape");
    CommonViews::ShaderParameter(settings->waveShape, settings->waveShapeOscillator);
    CommonViews::ShaderParameter(settings->harmonics, settings->harmonicsOscillator);
    CommonViews::ShaderParameter(settings->warpAmount, settings->warpAmountOscillator);
    CommonViews::ShaderParameter(settings->warpFreq, settings->warpFreqOscillator);
    CommonViews::ShaderParameter(settings->skew, settings->skewOscillator);
    CommonViews::ShaderParameter(settings->turbulence, settings->turbulenceOscillator);
    CommonViews::ShaderParameter(settings->detune, settings->detuneOscillator);
  }
};

#endif /* IridescentShader_hpp */
