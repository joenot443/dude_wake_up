//
//  AudioCymaticsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2024.
//

#ifndef AudioCymaticsShader_hpp
#define AudioCymaticsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioCymaticsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> modeCount;

  std::shared_ptr<Parameter> sharpness;
  std::shared_ptr<WaveformOscillator> sharpnessOscillator;

  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<WaveformOscillator> rotationOscillator;

  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;

  std::shared_ptr<Parameter> audioSensitivity;
  std::shared_ptr<WaveformOscillator> audioSensitivityOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> decay;
  std::shared_ptr<WaveformOscillator> decayOscillator;

  AudioCymaticsSettings(std::string shaderId, json j) :
  modeCount(std::make_shared<Parameter>("Mode Count", 6.0, 2.0, 12.0, ParameterType_Int)),
  sharpness(std::make_shared<Parameter>("Sharpness", 15.0, 1.0, 50.0)),
  sharpnessOscillator(std::make_shared<WaveformOscillator>(sharpness)),
  scale(std::make_shared<Parameter>("Scale", 2.0, 0.5, 4.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  rotation(std::make_shared<Parameter>("Rotation", 0.2, 0.0, 2.0)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, 1.0)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  audioSensitivity(std::make_shared<Parameter>("Audio Sensitivity", 2.0, 0.1, 5.0)),
  audioSensitivityOscillator(std::make_shared<WaveformOscillator>(audioSensitivity)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  decay(std::make_shared<Parameter>("Decay", 0.3, 0.0, 0.99)),
  decayOscillator(std::make_shared<WaveformOscillator>(decay)),
  ShaderSettings(shaderId, j, "AudioCymatics") {
    parameters = { modeCount, sharpness, scale, rotation, colorShift, audioSensitivity, brightness, decay };
    oscillators = { sharpnessOscillator, scaleOscillator, rotationOscillator, colorShiftOscillator, audioSensitivityOscillator, brightnessOscillator, decayOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioCymaticsShader: Shader {
  AudioCymaticsSettings *settings;
  AudioCymaticsShader(AudioCymaticsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/AudioCymatics");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1i("modeCount", (int)settings->modeCount->value);
    shader.setUniform1f("sharpness", settings->sharpness->value);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("rotation", settings->rotation->value);
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

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeAudioCymatics;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioCymatics");
    CommonViews::ShaderIntParameter(settings->modeCount);
    CommonViews::ShaderParameter(settings->sharpness, settings->sharpnessOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->audioSensitivity, settings->audioSensitivityOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
    CommonViews::ShaderParameter(settings->decay, settings->decayOscillator);
  }
};

#endif /* AudioCymaticsShader_hpp */
