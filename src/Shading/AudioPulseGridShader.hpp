//
//  AudioPulseGridShader.hpp
//  dude_wake_up
//

#ifndef AudioPulseGridShader_hpp
#define AudioPulseGridShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioPulseGridSettings: public ShaderSettings {
  std::shared_ptr<Parameter> columns;

  std::shared_ptr<Parameter> dotSize;
  std::shared_ptr<WaveformOscillator> dotSizeOscillator;

  std::shared_ptr<Parameter> pulseStrength;
  std::shared_ptr<WaveformOscillator> pulseStrengthOscillator;

  std::shared_ptr<Parameter> glowFalloff;
  std::shared_ptr<WaveformOscillator> glowFalloffOscillator;

  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> scrollSpeed;
  std::shared_ptr<WaveformOscillator> scrollSpeedOscillator;

  AudioPulseGridSettings(std::string shaderId, json j) :
  columns(std::make_shared<Parameter>("Columns", 16.0, 4.0, 32.0, ParameterType_Int)),
  dotSize(std::make_shared<Parameter>("Dot Size", 0.8, 0.1, 2.0)),
  dotSizeOscillator(std::make_shared<WaveformOscillator>(dotSize)),
  pulseStrength(std::make_shared<Parameter>("Pulse Strength", 1.5, 0.0, 4.0)),
  pulseStrengthOscillator(std::make_shared<WaveformOscillator>(pulseStrength)),
  glowFalloff(std::make_shared<Parameter>("Glow Falloff", 3.0, 1.0, 10.0)),
  glowFalloffOscillator(std::make_shared<WaveformOscillator>(glowFalloff)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, 1.0)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 2.0, 0.1, 5.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.2, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  scrollSpeed(std::make_shared<Parameter>("Scroll Speed", 0.5, 0.0, 3.0)),
  scrollSpeedOscillator(std::make_shared<WaveformOscillator>(scrollSpeed)),
  ShaderSettings(shaderId, j, "AudioPulseGrid") {
    parameters = { columns, dotSize, pulseStrength, glowFalloff, colorShift, audioSensitivity, brightness, scrollSpeed };
    oscillators = { dotSizeOscillator, pulseStrengthOscillator, glowFalloffOscillator, colorShiftOscillator, audioSensitivityOscillator, brightnessOscillator, scrollSpeedOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioPulseGridShader: Shader {
  AudioPulseGridSettings *settings;
  AudioPulseGridShader(AudioPulseGridSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/AudioPulseGrid");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1i("columns", (int)settings->columns->value);
    shader.setUniform1f("dotSize", settings->dotSize->value);
    shader.setUniform1f("pulseStrength", settings->pulseStrength->value);
    shader.setUniform1f("glowFalloff", settings->glowFalloff->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
    shader.setUniform1f("audioSensitivity", settings->audioSensitivity->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("decay", settings->scrollSpeed->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioPulseGrid; }

  void drawSettings() override {
    CommonViews::H3Title("AudioPulseGrid");
    CommonViews::ShaderIntParameter(settings->columns);
    CommonViews::ShaderParameter(settings->dotSize, settings->dotSizeOscillator);
    CommonViews::ShaderParameter(settings->pulseStrength, settings->pulseStrengthOscillator);
    CommonViews::ShaderParameter(settings->glowFalloff, settings->glowFalloffOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
    CommonViews::ShaderParameter(settings->scrollSpeed, settings->scrollSpeedOscillator);
  }
};

#endif
