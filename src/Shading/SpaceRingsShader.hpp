//
//  SpaceRingsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SpaceRingsShader_hpp
#define SpaceRingsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SpaceRingsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> particleSize;
  std::shared_ptr<WaveformOscillator> particleSizeOscillator;
  std::shared_ptr<Parameter> ringCount;
  std::shared_ptr<WaveformOscillator> ringCountOscillator;
  std::shared_ptr<Parameter> colorVariation;
  std::shared_ptr<WaveformOscillator> colorVariationOscillator;

  SpaceRingsSettings(std::string shaderId, json j) :
  particleSize(std::make_shared<Parameter>("Particle Size", 0.03, 0.01, 0.1)),
  particleSizeOscillator(std::make_shared<WaveformOscillator>(particleSize)),
  ringCount(std::make_shared<Parameter>("Ring Count", 5.0, 1.0, 10.0)),
  ringCountOscillator(std::make_shared<WaveformOscillator>(ringCount)),
  colorVariation(std::make_shared<Parameter>("Color Variation", 0.5, 0.0, 1.0)),
  colorVariationOscillator(std::make_shared<WaveformOscillator>(colorVariation)),
  ShaderSettings(shaderId, j, "SpaceRings") {
    parameters = { particleSize, ringCount, colorVariation };
    oscillators = { particleSizeOscillator, ringCountOscillator, colorVariationOscillator };
    load(j);
    registerParameters();
  };
};

struct SpaceRingsShader: Shader {
  SpaceRingsSettings *settings;
  SpaceRingsShader(SpaceRingsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/SpaceRings");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("particleSize", settings->particleSize->value);
    shader.setUniform1f("ringCount", settings->ringCount->value);
    shader.setUniform1f("colorVariation", settings->colorVariation->value);
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
    return ShaderTypeSpaceRings;
  }

  void drawSettings() override {
    CommonViews::H3Title("SpaceRings");

    CommonViews::ShaderParameter(settings->particleSize, settings->particleSizeOscillator);
    CommonViews::ShaderParameter(settings->ringCount, settings->ringCountOscillator);
    CommonViews::ShaderParameter(settings->colorVariation, settings->colorVariationOscillator);
  }
};

#endif
