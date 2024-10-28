//
//  TriangleMapShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TriangleMapShader_hpp
#define TriangleMapShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TriangleMapSettings: public ShaderSettings {
	public:
  // Speed Param
  std::shared_ptr<Parameter> speed;
  // Oscillator
  std::shared_ptr<Oscillator> speedOscillator;
  
  // Scale Param
  std::shared_ptr<Parameter> scale;
  // Oscillator
  std::shared_ptr<Oscillator> scaleOscillator;
  
  
  TriangleMapSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0., 5.0)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0., 5.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, scale};
    oscillators = {speedOscillator, scaleOscillator};
    load(j);
    registerParameters();
  };
};

class TriangleMapShader: public Shader {
public:

  TriangleMapSettings *settings;
  TriangleMapShader(TriangleMapSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
shader.load("shaders/TriangleMap");
shader.load("shaders/TriangleMap");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("speed", settings->speed->value);
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
    return ShaderTypeTriangleMap;
  }

  void drawSettings() override {
    CommonViews::H3Title("TriangleMap");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
  }
};

#endif
