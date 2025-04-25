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
  std::shared_ptr<Parameter> zoom; // New parameter
  std::shared_ptr<Parameter> brightness; // New parameter
  std::shared_ptr<Parameter> speed; // Add this line  
  std::shared_ptr<Parameter> color1; // Add this line
  std::shared_ptr<Parameter> color2; // Add this line
  std::shared_ptr<WaveformOscillator> zoomOscillator; // New oscillator
  std::shared_ptr<WaveformOscillator> brightnessOscillator; // New oscillator
  std::shared_ptr<WaveformOscillator> speedOscillator; // Add this line

  ColoredDropsSettings(std::string shaderId, json j) :
  zoom(std::make_shared<Parameter>("Zoom", 1.0, 0.1, 10.0)), // Initialize new parameter
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.0, 2.0)), // Initialize new parameter
  speed(std::make_shared<Parameter>("Speed", 0.5, 0.0, 2.0)), // Add speed initialization
  color1(std::make_shared<Parameter>("Color 1")), // Initialize color1
  color2(std::make_shared<Parameter>("Color 2")), // Initialize color2
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)), // Initialize new oscillator
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)), // Initialize new oscillator
  speedOscillator(std::make_shared<WaveformOscillator>(speed)), // Initialize speedOscillator
  ShaderSettings(shaderId, j, "Colored Drops") {
    // Default to purple and blue
    color1->setColor({0.6, 0.0, 1.0, 1.0});
    color2->setColor({0.0, 0.0, 1.0, 1.0});
    parameters = { zoom, brightness, speed, color1, color2 }; // Add speed, color1, color2
    oscillators = { zoomOscillator, brightnessOscillator, speedOscillator }; // Add speedOscillator
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
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform3f("color1", settings->color1->color->data()[0], settings->color1->color->data()[1], settings->color1->color->data()[2]);
    shader.setUniform3f("color2", settings->color2->color->data()[0], settings->color2->color->data()[1], settings->color2->color->data()[2]);
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
    

    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator); // Add zoom to drawSettings
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator); // Add brightness to drawSettings
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator); // Add speed control
    CommonViews::ShaderColor(settings->color1); // Add color1 control
    CommonViews::ShaderColor(settings->color2); // Add color2 control
  }
};

#endif
