//
//  ColoredDropsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColoredDropsShader_hpp
#define ColoredDropsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ColoredDropsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<Parameter> zoom; // New parameter
  std::shared_ptr<Parameter> brightness; // New parameter
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  std::shared_ptr<WaveformOscillator> zoomOscillator; // New oscillator
  std::shared_ptr<WaveformOscillator> brightnessOscillator; // New oscillator

  ColoredDropsSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  zoom(std::make_shared<Parameter>("zoom", 1.0, 0.1, 10.0)), // Initialize new parameter
  brightness(std::make_shared<Parameter>("brightness", 1.0, 0.0, 2.0)), // Initialize new parameter
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)), // Initialize new oscillator
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)), // Initialize new oscillator
  ShaderSettings(shaderId, j, "ColoredDrops") {
    parameters = { shaderValue, zoom, brightness }; // Add new parameters
    oscillators = { shaderValueOscillator, zoomOscillator, brightnessOscillator }; // Add new oscillators
    load(j);
    registerParameters();
  };
};

struct ColoredDropsShader: Shader {
  ColoredDropsSettings *settings;
  ColoredDropsShader(ColoredDropsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/ColoredDrops");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("zoom", settings->zoom->value); // Set uniform for zoom
    shader.setUniform1f("brightness", settings->brightness->value); // Set uniform for brightness
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
    return ShaderTypeColoredDrops;
  }

  void drawSettings() override {
    CommonViews::H3Title("ColoredDrops");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator); // Add zoom to drawSettings
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator); // Add brightness to drawSettings
  }
};

#endif /* ColoredDropsShader_hpp */
