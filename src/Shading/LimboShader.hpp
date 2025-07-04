//
//  LimboShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LimboShader_hpp
#define LimboShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct LimboSettings: public ShaderSettings {
  std::shared_ptr<Parameter> colorWarp;
  std::shared_ptr<WaveformOscillator> colorWarpOscillator;
  
  // Wave parameters
  std::shared_ptr<Parameter> waveFrequency;
  std::shared_ptr<Parameter> waveAmplitude;
  std::shared_ptr<WaveformOscillator> waveFrequencyOscillator;
  std::shared_ptr<WaveformOscillator> waveAmplitudeOscillator;
  
  // Ring parameters
  std::shared_ptr<Parameter> ringDensity;
  std::shared_ptr<Parameter> ringOffset;
  std::shared_ptr<WaveformOscillator> ringDensityOscillator;
  std::shared_ptr<WaveformOscillator> ringOffsetOscillator;
  
  // Color parameters
  std::shared_ptr<Parameter> colorSpeed;
  std::shared_ptr<Parameter> colorPhase;
  std::shared_ptr<WaveformOscillator> colorSpeedOscillator;
  std::shared_ptr<WaveformOscillator> colorPhaseOscillator;

  LimboSettings(std::string shaderId, json j, std::string name) :
  colorWarp(std::make_shared<Parameter>("Color Warp", 1.0, -1.0, 2.0)),
  colorWarpOscillator(std::make_shared<WaveformOscillator>(colorWarp)),
  
  // Wave parameters
  waveFrequency(std::make_shared<Parameter>("Wave Frequency", 0.25, 0.01, 1.0)),
  waveAmplitude(std::make_shared<Parameter>("Wave Amplitude", 2.0, 0.1, 5.0)),
  waveFrequencyOscillator(std::make_shared<WaveformOscillator>(waveFrequency)),
  waveAmplitudeOscillator(std::make_shared<WaveformOscillator>(waveAmplitude)),
  
  // Ring parameters
  ringDensity(std::make_shared<Parameter>("Ring Density", 1.0, 0.1, 3.0)),
  ringOffset(std::make_shared<Parameter>("Ring Offset", 0.0, 0.0, 1.0)),
  ringDensityOscillator(std::make_shared<WaveformOscillator>(ringDensity)),
  ringOffsetOscillator(std::make_shared<WaveformOscillator>(ringOffset)),
  
  // Color parameters
  colorSpeed(std::make_shared<Parameter>("Color Speed", 1.0, 0.1, 3.0)),
  colorPhase(std::make_shared<Parameter>("Color Phase", 0.0, 0.0, 1.0)),
  colorSpeedOscillator(std::make_shared<WaveformOscillator>(colorSpeed)),
  colorPhaseOscillator(std::make_shared<WaveformOscillator>(colorPhase)),
  
  ShaderSettings(shaderId, j, name) {
    parameters = { 
      colorWarp, waveFrequency, waveAmplitude, 
      ringDensity, ringOffset, colorSpeed, 
      colorPhase
    };
    
    oscillators = { 
      colorWarpOscillator, waveFrequencyOscillator, 
      waveAmplitudeOscillator, ringDensityOscillator, 
      ringOffsetOscillator, colorSpeedOscillator, 
      colorPhaseOscillator 
    };
    
    load(j);
    registerParameters();
  };
};

struct LimboShader: Shader {
  LimboSettings *settings;
  LimboShader(LimboSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Limbo");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("colorWarp", settings->colorWarp->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    // Wave uniforms
    shader.setUniform1f("waveFrequency", settings->waveFrequency->value);
    shader.setUniform1f("waveAmplitude", settings->waveAmplitude->value);
    
    // Ring uniforms
    shader.setUniform1f("ringDensity", settings->ringDensity->value);
    shader.setUniform1f("ringOffset", settings->ringOffset->value);
    
    // Color uniforms
    shader.setUniform1f("colorSpeed", settings->colorSpeed->value);
    shader.setUniform1f("colorPhase", settings->colorPhase->value);
    
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
    return ShaderTypeLimbo;
  }

  void drawSettings() override {
    // Wave controls
    ImGui::Text("Wave Controls");
    CommonViews::ShaderParameter(settings->waveFrequency, settings->waveFrequencyOscillator);
    CommonViews::ShaderParameter(settings->waveAmplitude, settings->waveAmplitudeOscillator);
    
    ImGui::Separator();
    
    // Ring controls
    ImGui::Text("Ring Controls");
    CommonViews::ShaderParameter(settings->ringDensity, settings->ringDensityOscillator);
    CommonViews::ShaderParameter(settings->ringOffset, settings->ringOffsetOscillator);
    
    ImGui::Separator();
    
    // Color controls
    ImGui::Text("Color Controls");
    CommonViews::ShaderParameter(settings->colorWarp, settings->colorWarpOscillator);
    CommonViews::ShaderParameter(settings->colorSpeed, settings->colorSpeedOscillator);
    CommonViews::ShaderParameter(settings->colorPhase, settings->colorPhaseOscillator);
  }
};

#endif
