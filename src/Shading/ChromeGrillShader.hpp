//
//  ChromeGrillShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ChromeGrillShader_hpp
#define ChromeGrillShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ChromeGrillSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;
  std::shared_ptr<WaveformOscillator> colorOscillator;
  
  std::shared_ptr<Parameter> warp;
  std::shared_ptr<WaveformOscillator> warpOscillator;

  ChromeGrillSettings(std::string shaderId, json j) :
  color(std::make_shared<Parameter>("color", 0.5, 0.0, 1.0)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  warp(std::make_shared<Parameter>("warp", 0.5, 0.0, 1.0)),
  warpOscillator(std::make_shared<WaveformOscillator>(warp)),
  ShaderSettings(shaderId, j, "ChromeGrill") {
    parameters = { color, warp };
    oscillators = { colorOscillator, warpOscillator };
    audioReactiveParameter = warp;
    load(j);
    registerParameters();
  };
};

struct ChromeGrillShader: Shader {
  ChromeGrillSettings *settings;
  ChromeGrillShader(ChromeGrillSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/ChromeGrill");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->color->value);
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
    return ShaderTypeChromeGrill;
  }

  void drawSettings() override {
    CommonViews::H3Title("ChromeGrill");

    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
    CommonViews::ShaderParameter(settings->warp, settings->warpOscillator);
  }
};

#endif /* ChromeGrillShader_hpp */
