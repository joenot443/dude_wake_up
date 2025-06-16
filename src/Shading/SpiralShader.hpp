//
//  SpiralShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SpiralShader_hpp
#define SpiralShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SpiralSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mainColor;
  std::shared_ptr<Parameter> secondaryColor;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> size;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> sizeOscillator;

  SpiralSettings(std::string shaderId, json j) :
  mainColor(std::make_shared<Parameter>("Main Color", 1.0, -1.0, 2.0)),
  secondaryColor(std::make_shared<Parameter>("Secondary Color", 1.0, -1.0, 2.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  size(std::make_shared<Parameter>("Size", 2.5, 0.1, 10.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  sizeOscillator(std::make_shared<WaveformOscillator>(size)),
  ShaderSettings(shaderId, j, "Spiral") {
    // Default colors
    mainColor->setColor({1.0, 1.0, 1.0, 1.0});     // White
    secondaryColor->setColor({0.0, 0.0, 0.0, 1.0}); // Black
    parameters = { speed, size };
    oscillators = { speedOscillator, sizeOscillator };
    load(j);
    registerParameters();
  };
};

struct SpiralShader: Shader {
  SpiralSettings *settings;
  SpiralShader(SpiralSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Spiral");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
    shader.setUniform1f("size", settings->size->value);
    shader.setUniform1f("speed", 1.0);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform3f("mainColor",
      settings->mainColor->color->data()[0],
      settings->mainColor->color->data()[1],
      settings->mainColor->color->data()[2]);
    shader.setUniform3f("secondaryColor",
      settings->secondaryColor->color->data()[0],
      settings->secondaryColor->color->data()[1],
      settings->secondaryColor->color->data()[2]);
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
    return ShaderTypeSpiral;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
    CommonViews::ShaderColor(settings->mainColor);
    CommonViews::ShaderColor(settings->secondaryColor);
  }
};

#endif /* SpiralShader_hpp */
