//
//  StellarShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StellarShader_hpp
#define StellarShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StellarSettings: public ShaderSettings {
  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<WaveformOscillator> thresholdOscillator;

  std::shared_ptr<Parameter> decay;
  std::shared_ptr<WaveformOscillator> decayOscillator;

  std::shared_ptr<Parameter> density;
  std::shared_ptr<WaveformOscillator> densityOscillator;

  std::shared_ptr<Parameter> weight;
  std::shared_ptr<WaveformOscillator> weightOscillator;

  std::shared_ptr<Parameter> centerX;
  std::shared_ptr<WaveformOscillator> centerXOscillator;

  std::shared_ptr<Parameter> centerY;
  std::shared_ptr<WaveformOscillator> centerYOscillator;

  StellarSettings(std::string shaderId, json j) :
  threshold(std::make_shared<Parameter>("Threshold", 0.7, 0.0, 1.0)),
  thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),
  decay(std::make_shared<Parameter>("Decay", 1.0, 0.9, 1.0)),
  decayOscillator(std::make_shared<WaveformOscillator>(decay)),
  density(std::make_shared<Parameter>("Density", 0.9, 0.1, 2.0)),
  densityOscillator(std::make_shared<WaveformOscillator>(density)),
  weight(std::make_shared<Parameter>("Weight", 0.05, 0.0, 0.2)),
  weightOscillator(std::make_shared<WaveformOscillator>(weight)),
  centerX(std::make_shared<Parameter>("Center X", 0.5, 0.0, 1.0)),
  centerXOscillator(std::make_shared<WaveformOscillator>(centerX)),
  centerY(std::make_shared<Parameter>("Center Y", 0.5, 0.0, 1.0)),
  centerYOscillator(std::make_shared<WaveformOscillator>(centerY)),
  ShaderSettings(shaderId, j, "Stellar") {
    parameters = { threshold, decay, density, weight, centerX, centerY };
    oscillators = { thresholdOscillator, decayOscillator, densityOscillator, weightOscillator, centerXOscillator, centerYOscillator };
    load(j);
    registerParameters();
  };
};

struct StellarShader: Shader {
  StellarSettings *settings;
  StellarShader(StellarSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Stellar");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("threshold", settings->threshold->value);
    shader.setUniform1f("decay", settings->decay->value);
    shader.setUniform1f("density", settings->density->value);
    shader.setUniform1f("weight", settings->weight->value);
    shader.setUniform2f("center", settings->centerX->value, settings->centerY->value);
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
    return ShaderTypeStellar;
  }

  void drawSettings() override {
    CommonViews::H3Title("Stellar");
    CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
    CommonViews::ShaderParameter(settings->decay, settings->decayOscillator);
    CommonViews::ShaderParameter(settings->density, settings->densityOscillator);
    CommonViews::ShaderParameter(settings->weight, settings->weightOscillator);
    CommonViews::ShaderParameter(settings->centerX, settings->centerXOscillator);
    CommonViews::ShaderParameter(settings->centerY, settings->centerYOscillator);
  }
};

#endif /* StellarShader_hpp */
