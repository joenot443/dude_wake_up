//
//  PerplexionShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PerplexionShader_hpp
#define PerplexionShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PerplexionSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<WaveformOscillator> alphaOscillator;
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<WaveformOscillator> betaOscillator;
  std::shared_ptr<Parameter> gamma;
  std::shared_ptr<WaveformOscillator> gammaOscillator;
  
  PerplexionSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 10.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 5.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  beta(std::make_shared<Parameter>("Beta", 1.0, 0.0, 5.0)),
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),
  gamma(std::make_shared<Parameter>("Gamma", 1.0, 0.0, 5.0)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  ShaderSettings(shaderId, j, "Perplexion") {
    
    parameters = { speed, alpha, beta, gamma };
    oscillators = { speedOscillator, alphaOscillator, betaOscillator, gammaOscillator };
    load(j);
    registerParameters();
  };
};

struct PerplexionShader: Shader {
  PerplexionSettings *settings;
  PerplexionShader(PerplexionSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Perplexion");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("alpha", settings->alpha->value);
    shader.setUniform1f("beta", settings->beta->value);
    shader.setUniform1f("gamma", settings->gamma->value);
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
    return ShaderTypePerplexion;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
  }
};

#endif
