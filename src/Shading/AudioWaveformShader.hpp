//
//  AudioWaveformShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioWaveformShader_hpp
#define AudioWaveformShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "AudioSourceService.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

class AudioWaveformSettings: public ShaderSettings {
public:
  std::string shaderId;

  std::shared_ptr<Parameter> thickness;
  std::shared_ptr<Parameter> lineCount;
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<Parameter> glowIntensity;

  std::shared_ptr<Oscillator> thicknessOscillator;
  std::shared_ptr<Oscillator> amplitudeOscillator;
  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> colorShiftOscillator;
  std::shared_ptr<Oscillator> glowIntensityOscillator;

  AudioWaveformSettings(std::string shaderId, json j, std::string name) :
  thickness(std::make_shared<Parameter>("Thickness", 1.0, 0.5, 5.0)),
  lineCount(std::make_shared<Parameter>("Line Count", 5.0, 1.0, 15.0, ParameterType_Int)),
  amplitude(std::make_shared<Parameter>("Amplitude", 1.0, 0.0, 3.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, 0.0, TWO_PI)),
  glowIntensity(std::make_shared<Parameter>("Glow Intensity", 1.0, 0.1, 3.0)),
  thicknessOscillator(std::make_shared<WaveformOscillator>(thickness)),
  amplitudeOscillator(std::make_shared<WaveformOscillator>(amplitude)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  glowIntensityOscillator(std::make_shared<WaveformOscillator>(glowIntensity)),
  shaderId(shaderId),
  ShaderSettings(shaderId, j, name) {
    parameters = {thickness, lineCount, amplitude, speed, colorShift, glowIntensity};
    oscillators = {thicknessOscillator, amplitudeOscillator, speedOscillator, colorShiftOscillator, glowIntensityOscillator};
    audioReactiveParameter = amplitude;
    load(j);
    registerParameters();
  };
};

class AudioWaveformShader: public Shader {
public:
  
  AudioWaveformSettings *settings;
  AudioWaveformShader(AudioWaveformSettings *settings) :
  settings(settings),
  Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/AudioWaveform");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value * settings->speed->value);
    shader.setUniform1f("thickness", settings->thickness->value);
    shader.setUniform1i("lineCount", settings->lineCount->intValue);
    shader.setUniform1f("amplitude", settings->amplitude->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
    shader.setUniform1f("glowIntensity", settings->glowIntensity->value);
    if (source != nullptr) {
      std::vector<float> safeSpectrum = source->audioAnalysis.getSafeSpectrum();
      if (safeSpectrum.size() >= 256)
        shader.setUniform1fv("audio", &safeSpectrum[0], 256);
      std::vector<float> safeWaveform = source->audioAnalysis.getSafeWaveform();
      if (safeWaveform.size() >= 256)
        shader.setUniform1fv("waveform", &safeWaveform[0], 256);
    }
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
    return ShaderTypeAudioWaveform;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->thickness, settings->thicknessOscillator);
    CommonViews::ShaderIntParameter(settings->lineCount);
    CommonViews::ShaderParameter(settings->amplitude, settings->amplitudeOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->glowIntensity, settings->glowIntensityOscillator);
  }
};

#endif
