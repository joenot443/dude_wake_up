//
//  MelterShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/2/22.
//

#ifndef MelterShader_h
#define MelterShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MeltSettings : public ShaderSettings {
	public:
  std::string shaderId;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<WaveformOscillator> alphaOscillator;
  
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<WaveformOscillator> betaOscillator;
  
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;


  MeltSettings(std::string shaderId, json j, std::string name) : shaderId(shaderId),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 2.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  beta(std::make_shared<Parameter>("Beta", 1.0, 0.0, 2.0)),
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j, name) {
    load(j);
  registerParameters();
  };
};

class MeltShader : public Shader {
public:

  private:
  ofShader shader;

  public:
  MeltSettings *settings;

  MeltShader(MeltSettings *settings) : settings(settings),
  Shader(settings) {
    #ifdef TESTING
shader.load("shaders/Melt");
#endif
#ifdef RELEASE
shader.load("shaders/Melt");
#endif
  }
  
  int inputCount() override {
    return 0;
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("alpha", settings->alpha->value);
    shader.setUniform1f("beta", settings->beta->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    
    CommonViews::H3Title("Melt");
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};
#endif /* MelterShader_h */
