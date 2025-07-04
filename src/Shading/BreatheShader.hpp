//
//  BreatheShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BreatheShader_hpp
#define BreatheShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BreatheSettings: public ShaderSettings {
  std::shared_ptr<Parameter> pulseIntensity;
  std::shared_ptr<Parameter> warpSpeed;
  std::shared_ptr<Parameter> colorShift;
  std::shared_ptr<WaveformOscillator> pulseIntensityOscillator;
  std::shared_ptr<WaveformOscillator> warpSpeedOscillator;
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;
  
  BreatheSettings(std::string shaderId, json j) :
  pulseIntensity(std::make_shared<Parameter>("Zoom", 1.0, 0.1, 2.0)),
  warpSpeed(std::make_shared<Parameter>("Warp Speed", 1.0, 0.1, 30.0)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, -1.0, 1.0)),
  pulseIntensityOscillator(std::make_shared<WaveformOscillator>(pulseIntensity)),
  warpSpeedOscillator(std::make_shared<WaveformOscillator>(warpSpeed)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  ShaderSettings(shaderId, j, "Breathe") {
    parameters = { pulseIntensity, warpSpeed, colorShift };
    oscillators = { pulseIntensityOscillator, warpSpeedOscillator, colorShiftOscillator };
    load(j);
    registerParameters();
  };
};

struct BreatheShader: Shader {
  BreatheSettings *settings;
  BreatheShader(BreatheSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Breathe");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("pulseIntensity", settings->pulseIntensity->value);
    shader.setUniform1f("warpSpeed", settings->warpSpeed->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
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
    return ShaderTypeBreathe;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->pulseIntensity, settings->pulseIntensityOscillator);
    CommonViews::ShaderParameter(settings->warpSpeed, settings->warpSpeedOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
  }
};

#endif
