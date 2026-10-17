//
//  AudioAuroraShader.hpp
//  dude_wake_up
//

#ifndef AudioAuroraShader_hpp
#define AudioAuroraShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioAuroraSettings: public ShaderSettings {
  std::shared_ptr<Parameter> ribbonCount;

  std::shared_ptr<Parameter> waveSpeed;
  std::shared_ptr<WaveformOscillator> waveSpeedOscillator;

  std::shared_ptr<Parameter> height;
  std::shared_ptr<WaveformOscillator> heightOscillator;

  std::shared_ptr<Parameter> thickness;
  std::shared_ptr<WaveformOscillator> thicknessOscillator;

  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> decay;
  std::shared_ptr<WaveformOscillator> decayOscillator;

  AudioAuroraSettings(std::string shaderId, json j) :
  ribbonCount(std::make_shared<Parameter>("Ribbon Count", 5.0, 2.0, 10.0, ParameterType_Int)),
  waveSpeed(std::make_shared<Parameter>("Wave Speed", 1.0, 0.0, 3.0)),
  waveSpeedOscillator(std::make_shared<WaveformOscillator>(waveSpeed)),
  height(std::make_shared<Parameter>("Height", 0.5, 0.1, 1.0)),
  heightOscillator(std::make_shared<WaveformOscillator>(height)),
  thickness(std::make_shared<Parameter>("Thickness", 0.08, 0.01, 0.3)),
  thicknessOscillator(std::make_shared<WaveformOscillator>(thickness)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, 1.0)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 3.0, 0.1, 8.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  decay(std::make_shared<Parameter>("Decay", 0.4, 0.0, 0.99)),
  decayOscillator(std::make_shared<WaveformOscillator>(decay)),
  ShaderSettings(shaderId, j, "AudioAurora") {
    parameters = { ribbonCount, waveSpeed, height, thickness, colorShift, audioSensitivity, brightness, decay };
    oscillators = { waveSpeedOscillator, heightOscillator, thicknessOscillator, colorShiftOscillator, audioSensitivityOscillator, brightnessOscillator, decayOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioAuroraShader: Shader {
  AudioAuroraSettings *settings;
  AudioAuroraShader(AudioAuroraSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/AudioAurora");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1i("ribbonCount", (int)settings->ribbonCount->value);
    shader.setUniform1f("waveSpeed", settings->waveSpeed->value);
    shader.setUniform1f("height", settings->height->value);
    shader.setUniform1f("thickness", settings->thickness->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
    shader.setUniform1f("audioSensitivity", settings->audioSensitivity->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("decay", settings->decay->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}
  int inputCount() override { return 1; }
  ShaderType type() override { return ShaderTypeAudioAurora; }

  void drawSettings() override {
    CommonViews::H3Title("AudioAurora");
    CommonViews::ShaderIntParameter(settings->ribbonCount);
    CommonViews::ShaderParameter(settings->waveSpeed, settings->waveSpeedOscillator);
    CommonViews::ShaderParameter(settings->height, settings->heightOscillator);
    CommonViews::ShaderParameter(settings->thickness, settings->thicknessOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
    CommonViews::ShaderParameter(settings->decay, settings->decayOscillator);
  }
};

#endif
