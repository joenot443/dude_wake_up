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
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<Parameter> alpha; // New parameter
  std::shared_ptr<Parameter> beta;  // New parameter
  std::shared_ptr<Parameter> gamma; // New parameter
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  std::shared_ptr<WaveformOscillator> alphaOscillator; // New oscillator
  std::shared_ptr<WaveformOscillator> betaOscillator;  // New oscillator
  std::shared_ptr<WaveformOscillator> gammaOscillator; // New oscillator

  PlasmorSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  alpha(std::make_shared<Parameter>("alpha", 0.0, 0.0, 1.0)), // Initialize new parameter
  beta(std::make_shared<Parameter>("beta", 0.5, 0.0, 1.0)),  // Initialize new parameter
  gamma(std::make_shared<Parameter>("gamma", 0.5, 0.0, 1.0)), // Initialize new parameter
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)), // Initialize new oscillator
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),  // Initialize new oscillator
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)), // Initialize new oscillator
  ShaderSettings(shaderId, j, "Plasmor") {
    parameters = { shaderValue, alpha, beta, gamma }; // Add new parameters
    oscillators = { shaderValueOscillator, alphaOscillator, betaOscillator, gammaOscillator }; // Add new oscillators
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
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("alpha", settings->alpha->value); // Set uniform for alpha
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
    CommonViews::H3Title("Plasmor");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator); // Add alpha to drawSettings
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);   // Add beta to drawSettings
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator); // Add gamma to drawSettings
  }
};

#endif /* PlasmorShader_hpp */
