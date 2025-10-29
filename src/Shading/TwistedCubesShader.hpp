//
//  TwistedCubesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TwistedCubesShader_hpp
#define TwistedCubesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TwistedCubesSettings: public ShaderSettings {
  std::shared_ptr<Parameter> spacing;
  std::shared_ptr<Parameter> shape;
  std::shared_ptr<WaveformOscillator> spacingOscillator;
  std::shared_ptr<WaveformOscillator> shapeOscillator;

  TwistedCubesSettings(std::string shaderId, json j) :
  spacing(std::make_shared<Parameter>("Spacing", 0.25, 0.01, 0.5)),
  shape(std::make_shared<Parameter>("Shape", 1.0, 0.5, 1.0)),
  shapeOscillator(std::make_shared<WaveformOscillator>(shape)),
  spacingOscillator(std::make_shared<WaveformOscillator>(spacing)),
  ShaderSettings(shaderId, j, "TwistedCubes") {
    parameters = { spacing, shape };
    oscillators = { spacingOscillator, shapeOscillator };
    load(j);
    registerParameters();
  };
};

struct TwistedCubesShader: Shader {
  TwistedCubesSettings *settings;
  TwistedCubesShader(TwistedCubesSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/TwistedCubes");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("shape", settings->shape->value);
    shader.setUniform1f("spacing", settings->spacing->value);
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
    return ShaderTypeTwistedCubes;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->spacing, settings->spacingOscillator);
    CommonViews::ShaderParameter(settings->shape, settings->shapeOscillator);
  }
};

#endif
