//
//  PlasmaShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef PlasmaShader_h
#define PlasmaShader_h

#include <stdio.h>
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"

#include "CommonViews.hpp"

// Plasma

struct PlasmaSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<Parameter> gamma;
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<Parameter> delta;

  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> colorOscillator;
  std::shared_ptr<WaveformOscillator> alphaOscillator;
  std::shared_ptr<WaveformOscillator> gammaOscillator;
  std::shared_ptr<WaveformOscillator> betaOscillator;
  std::shared_ptr<WaveformOscillator> deltaOscillator;

  PlasmaSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("Speed", 1.0,  0.0, 5.0)),
  color(std::make_shared<Parameter>("Color", 1.0,  0.0, 1.0)),
  alpha(std::make_shared<Parameter>("Alpha", 0.5, 0.0, 1.0)),
  gamma(std::make_shared<Parameter>("Gamma", 1.0, 0.0, 3.0)),
  beta(std::make_shared<Parameter>("Beta", 1.0, 0.0, 3.0)),
  delta(std::make_shared<Parameter>("Delta", 1.0, 0.0, 3.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  betaOscillator(std::make_shared<WaveformOscillator>(beta)),
  deltaOscillator(std::make_shared<WaveformOscillator>(delta)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, color, alpha, gamma, beta, delta};
    oscillators = {speedOscillator, colorOscillator, alphaOscillator, gammaOscillator, betaOscillator, deltaOscillator};
    load(j);
    registerParameters();
   };
};

class PlasmaShader : public Shader {
private:


public:
 PlasmaShader(PlasmaSettings* settings) :
  settings(settings),
  Shader(settings) {};
  
  PlasmaSettings *settings;
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    frame->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    canvas->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform2f("sourceDimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("color", settings->color->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("alpha", settings->alpha->value);
    shader.setUniform1f("gamma", settings->gamma->value);
    shader.setUniform1f("beta", settings->beta->value);
    shader.setUniform1f("delta", settings->delta->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shaders/Plasma");
  };
  
  int inputCount() override {
    return 0;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
    CommonViews::ShaderParameter(settings->beta, settings->betaOscillator);
    CommonViews::ShaderParameter(settings->delta, settings->deltaOscillator);
  };
};


#endif
