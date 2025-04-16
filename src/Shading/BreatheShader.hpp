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
  std::shared_ptr<Parameter> pulseIntensity; // New parameter
  std::shared_ptr<Parameter> warpSpeed;      // New parameter
  std::shared_ptr<Parameter> colorShift;     // New parameter
  std::shared_ptr<WaveformOscillator> pulseIntensityOscillator; // New oscillator
  std::shared_ptr<WaveformOscillator> warpSpeedOscillator;      // New oscillator
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;     // New oscillator

  BreatheSettings(std::string shaderId, json j) :
  pulseIntensity(std::make_shared<Parameter>("Zoom", 1.0, 0.1, 2.0)), // Initialize new parameter
  warpSpeed(std::make_shared<Parameter>("Warp Speed", 1.0, 0.1, 10.0)),           // Initialize new parameter
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, -1.0, 1.0)),        // Initialize new parameter
  pulseIntensityOscillator(std::make_shared<WaveformOscillator>(pulseIntensity)), // Initialize new oscillator
  warpSpeedOscillator(std::make_shared<WaveformOscillator>(warpSpeed)),           // Initialize new oscillator
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),         // Initialize new oscillator
  ShaderSettings(shaderId, j, "Breathe") {
    parameters = { pulseIntensity, warpSpeed, colorShift }; // Add new parameters
    oscillators = { pulseIntensityOscillator, warpSpeedOscillator, colorShiftOscillator }; // Add new oscillators
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
    shader.setUniform1f("pulseIntensity", settings->pulseIntensity->value); // Set uniform for pulseIntensity
    shader.setUniform1f("warpSpeed", settings->warpSpeed->value);           // Set uniform for warpSpeed
    shader.setUniform1f("colorShift", settings->colorShift->value);         // Set uniform for colorShift
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    

    CommonViews::ShaderParameter(settings->pulseIntensity, settings->pulseIntensityOscillator); // Add pulseIntensity to drawSettings
    CommonViews::ShaderParameter(settings->warpSpeed, settings->warpSpeedOscillator);           // Add warpSpeed to drawSettings
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);         // Add colorShift to drawSettings
  }
};

#endif
