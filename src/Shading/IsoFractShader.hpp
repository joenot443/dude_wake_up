//
//  IsoFractShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef IsoFractShader_hpp
#define IsoFractShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct IsoFractSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<WaveformOscillator> alphaOscillator;
  
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<WaveformOscillator> betaOscillator;
  
  std::shared_ptr<Parameter> gamma;
  std::shared_ptr<WaveformOscillator> gammaOscillator;

  IsoFractSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  alpha(std::make_shared<Parameter>("alpha", 1.0, 0.0, 5.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  beta(std::make_shared<Parameter>("beta", 2.0, -2.0, 2.0)),
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),
  gamma(std::make_shared<Parameter>("gamma", -2.0, -2.0, 2.0)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  
  ShaderSettings(shaderId, j, "IsoFract") {
    parameters = { alpha, beta, gamma, shaderValue };
    oscillators = { alphaOscillator, betaOscillator, gammaOscillator, shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct IsoFractShader: Shader {
  IsoFractSettings *settings;
  IsoFractShader(IsoFractSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/IsoFract");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override;

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeIsoFract;
  }

  void drawSettings() override {
    CommonViews::H3Title("IsoFract");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
  }
};

#endif /* IsoFractShader_hpp */
