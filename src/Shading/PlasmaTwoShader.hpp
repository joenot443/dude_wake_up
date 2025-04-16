//
//  PlasmaTwoShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PlasmaTwoShader_hpp
#define PlasmaTwoShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PlasmaTwoSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<WaveformOscillator> mixOscillator;
  
  PlasmaTwoSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("Color", 1.0  , -1.0, 2.0)),
  mix(std::make_shared<Parameter>("Color Mix", 0.0, 0.0, 1.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId, j, name)
  {
    parameters = { color, mix };
    oscillators = { mixOscillator };
    load(j);
    registerParameters();
  };
};

class PlasmaTwoShader : public Shader
{
public:
  PlasmaTwoSettings *settings;
  PlasmaTwoShader(PlasmaTwoSettings *settings) : settings(settings), Shader(settings){};

  void setup() override
  {
    shader.load("shaders/PlasmaTwo");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->color->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("colorMix", settings->mix->value);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { 
    return ShaderTypePlasmaTwo;
  }

  void drawSettings() override
  {
    
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
  }
};

#endif
