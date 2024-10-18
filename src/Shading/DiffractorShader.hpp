//
//  DiffractorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiffractorShader_hpp
#define DiffractorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DiffractorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> spacing;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> count;
  std::shared_ptr<Parameter> scramble;
  std::shared_ptr<WaveformOscillator> spacingOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> scrambleOscillator;
  
  DiffractorSettings(std::string shaderId, json j) :
  spacing(std::make_shared<Parameter>("spacing", 0.2, 0.0, 1.0)),
  speed(std::make_shared<Parameter>("speed", 5.0, 0.0, 10.0)),
  scale(std::make_shared<Parameter>("scale", 1.0, 0.1, 10.0)),
  count(std::make_shared<Parameter>("count", 5, 1, 10)),
  scramble(std::make_shared<Parameter>("scramble", 0.5, 0.0, 1.0)), // Initialize new parameter
  spacingOscillator(std::make_shared<WaveformOscillator>(spacing)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  scrambleOscillator(std::make_shared<WaveformOscillator>(scramble)), // Initialize new oscillator
  ShaderSettings(shaderId, j, "Diffractor") {
    parameters = { spacing, speed, scale, count, scramble }; // Add new parameter
    oscillators = { spacingOscillator, speedOscillator, scaleOscillator, scrambleOscillator }; // Add new oscillator
    load(j);
    registerParameters();
  };
};

struct DiffractorShader: Shader {
  DiffractorSettings *settings;
  DiffractorShader(DiffractorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Diffractor");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("spacing", settings->spacing->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1i("count", settings->count->intValue);
    shader.setUniform1f("scramble", settings->scramble->value); // Set uniform for scramble
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
    return ShaderTypeDiffractor;
  }

  void drawSettings() override {
    CommonViews::H3Title("Diffractor");

    CommonViews::ShaderParameter(settings->spacing, settings->spacingOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderIntParameter(settings->count);
    CommonViews::ShaderParameter(settings->scramble, settings->scrambleOscillator); // Add scramble to drawSettings
  }
};

#endif /* DiffractorShader_hpp */
