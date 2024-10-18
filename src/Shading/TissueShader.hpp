//
//  TissueShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TissueShader_hpp
#define TissueShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct TissueSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<Parameter> gamma;
  
  std::shared_ptr<Oscillator> alphaOscillator;
  std::shared_ptr<Oscillator> betaOscillator;
  std::shared_ptr<Oscillator> gammaOscillator;
  
  TissueSettings(std::string shaderId, json j, std::string name)
  : alpha(std::make_shared<Parameter>("alpha", 3, 0., 10.)),
  beta(std::make_shared<Parameter>("beta", 7, 0., 10.)),
  gamma(std::make_shared<Parameter>("gamma", 4, 0., 10.)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  ShaderSettings(shaderId, j, name){
    
  };
};

class TissueShader : public Shader {
public:

  TissueSettings *settings;
  TissueShader(TissueSettings *settings)
  : settings(settings), Shader(settings){};

  void setup() override {
#ifdef DEBUG
    shader.load("shaders/Tissue");
#endif
#ifdef RELEASE
    shader.load("shaders/Tissue");
#endif
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    shader.setUniform1f("alpha", settings->alpha->value);
    shader.setUniform1f("beta", settings->beta->value);
    shader.setUniform1f("gamma", settings->gamma->value);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypeTissue; }
  
  void drawSettings() override {
    
    CommonViews::H3Title("Tissue");
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
  }
};

#endif /* TissueShader_hpp */
