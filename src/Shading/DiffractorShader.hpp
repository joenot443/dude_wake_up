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
  std::shared_ptr<Parameter> red;
  std::shared_ptr<Parameter> green;
  std::shared_ptr<Parameter> blue;
  std::shared_ptr<WaveformOscillator> spacingOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> scrambleOscillator;
  
  DiffractorSettings(std::string shaderId, json j) :
  spacing(std::make_shared<Parameter>("Spacing", 0.2, 0.0, 1.0)),
  speed(std::make_shared<Parameter>("Speed", 5.0, 0.0, 10.0)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.1, 10.0)),
  count(std::make_shared<Parameter>("Count", 5, 1, 10)),
  scramble(std::make_shared<Parameter>("Scramble", 0.5, 0.0, 1.0)),
  red(std::make_shared<Parameter>("Red", 1.0, 0.0, 1.0)),
  green(std::make_shared<Parameter>("Green", 1.0, 0.0, 1.0)),
  blue(std::make_shared<Parameter>("Blue", 1.0, 0.0, 1.0)),
  spacingOscillator(std::make_shared<WaveformOscillator>(spacing)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  scrambleOscillator(std::make_shared<WaveformOscillator>(scramble)),
  ShaderSettings(shaderId, j, "Diffractor") {
    parameters = { spacing, speed, scale, count, scramble, red, green, blue };
    oscillators = { spacingOscillator, speedOscillator, scaleOscillator, scrambleOscillator }; 
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
    shader.setUniform1f("red", settings->red->value);
    shader.setUniform1f("green", settings->green->value);
    shader.setUniform1f("blue", settings->blue->value);
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
    CommonViews::ShaderParameter(settings->spacing, settings->spacingOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderIntParameter(settings->count);
    CommonViews::ShaderParameter(settings->scramble, settings->scrambleOscillator); // Add scramble to drawSettings
    CommonViews::ShaderParameter(settings->red, nullptr);
    CommonViews::ShaderParameter(settings->green, nullptr);
    CommonViews::ShaderParameter(settings->blue, nullptr);
  }
};

#endif
