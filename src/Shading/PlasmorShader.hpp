//
//  PlasmorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PlasmorShader_hpp
#define PlasmorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PlasmorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> beta;  // New parameter
  std::shared_ptr<Parameter> gamma; // New parameter
  std::shared_ptr<WaveformOscillator> betaOscillator;  // New oscillator
  std::shared_ptr<WaveformOscillator> gammaOscillator; // New oscillator

  PlasmorSettings(std::string shaderId, json j) :
  beta(std::make_shared<Parameter>("Beta", 0.5, 0.0, 1.0)),  // Initialize new parameter
  gamma(std::make_shared<Parameter>("Gamma", 0.5, 0.0, 1.0)), // Initialize new parameter
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),  // Initialize new oscillator
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)), // Initialize new oscillator
  ShaderSettings(shaderId, j, "Plasmor") {
    parameters = { beta, gamma }; // Add new parameters
    oscillators = {betaOscillator, gammaOscillator }; // Add new oscillators
    load(j);
    registerParameters();
  };
};

struct PlasmorShader: Shader {
  PlasmorSettings *settings;
  PlasmorShader(PlasmorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Plasmor");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform1f("beta", settings->beta->value);   // Set uniform for beta
    shader.setUniform1f("gamma", settings->gamma->value); // Set uniform for gamma
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
    return ShaderTypePlasmor;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);   // Add beta to drawSettings
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator); // Add gamma to drawSettings
  }
};

#endif
