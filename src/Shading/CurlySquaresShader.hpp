//
//  CurlySquaresShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CurlySquaresShader_hpp
#define CurlySquaresShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CurlySquaresSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> size;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<Parameter> beta;
  std::shared_ptr<Parameter> gamma;
  
  std::shared_ptr<WaveformOscillator> sizeOscillator;
  
  
  CurlySquaresSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("Color", 1.0, -1.0, 2.0)),
  size(std::make_shared<Parameter>("Size", 1.0, 0.0, 2.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 2.0)),
  beta(std::make_shared<Parameter>("Beta", 1.0, 0.0, 2.0)),
  gamma(std::make_shared<Parameter>("Gamma", 4.0, 0.0, 20.0)),
  sizeOscillator(std::make_shared<WaveformOscillator>(size)),
  ShaderSettings(shaderId, j, name) {
    parameters = { color, size, alpha, beta, gamma };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

class CurlySquaresShader: public Shader {
public:
  CurlySquaresSettings *settings;
  CurlySquaresShader(CurlySquaresSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/CurlySquares");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("size", settings->size->value);
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
    return ShaderTypeCurlySquares;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("CurlySquares");
    
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
    CommonViews::ShaderParameter(settings->alpha, nullptr);
    CommonViews::ShaderParameter(settings->beta, nullptr);
    CommonViews::ShaderParameter(settings->gamma, nullptr);
  }
};

#endif
