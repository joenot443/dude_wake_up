//
//  FloatingSparksShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FloatingSparksShader_hpp
#define FloatingSparksShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FloatingSparksSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> distancePower;
  std::shared_ptr<WaveformOscillator> distancePowerOscillator;

  std::shared_ptr<Parameter> radius;
  std::shared_ptr<WaveformOscillator> radiusOscillator;

  std::shared_ptr<Parameter> redMultiplier;
  std::shared_ptr<WaveformOscillator> redMultiplierOscillator;

  std::shared_ptr<Parameter> greenMultiplier;
  std::shared_ptr<WaveformOscillator> greenMultiplierOscillator;

  std::shared_ptr<Parameter> blueMultiplier;
  std::shared_ptr<WaveformOscillator> blueMultiplierOscillator;

  FloatingSparksSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("Shader Value", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  distancePower(std::make_shared<Parameter>("Glow", 0.2, 0.0, 1.0)),
  distancePowerOscillator(std::make_shared<WaveformOscillator>(distancePower)),
  radius(std::make_shared<Parameter>("Radius", 0.5, 0.0, 2.0)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  redMultiplier(std::make_shared<Parameter>("Red", 1.0, 0.0, 2.0)),
  redMultiplierOscillator(std::make_shared<WaveformOscillator>(redMultiplier)),
  greenMultiplier(std::make_shared<Parameter>("Green", 1.0, 0.0, 2.0)),
  greenMultiplierOscillator(std::make_shared<WaveformOscillator>(greenMultiplier)),
  blueMultiplier(std::make_shared<Parameter>("Blue", 1.0, 0.0, 2.0)),
  blueMultiplierOscillator(std::make_shared<WaveformOscillator>(blueMultiplier)),
  ShaderSettings(shaderId, j, "Floating Sparks") {
    parameters = { shaderValue, distancePower, radius, redMultiplier, greenMultiplier, blueMultiplier };
    oscillators = { shaderValueOscillator, distancePowerOscillator, radiusOscillator, redMultiplierOscillator, greenMultiplierOscillator, blueMultiplierOscillator };
    load(j);
    registerParameters();
  };
};

struct FloatingSparksShader: Shader {
  FloatingSparksSettings *settings;
  FloatingSparksShader(FloatingSparksSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Floating Sparks");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("distancePower", settings->distancePower->value);
    shader.setUniform1f("radiusMultiplier", settings->radius->value);
    shader.setUniform1f("redMultiplier", settings->redMultiplier->value * 0.7);
    shader.setUniform1f("greenMultiplier", settings->greenMultiplier->value);
    shader.setUniform1f("blueMultiplier", settings->blueMultiplier->value);
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
    return ShaderTypeFloatingSparks;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->distancePower, settings->distancePowerOscillator);
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
    CommonViews::ShaderParameter(settings->redMultiplier, settings->redMultiplierOscillator);
    CommonViews::ShaderParameter(settings->greenMultiplier, settings->greenMultiplierOscillator);
    CommonViews::ShaderParameter(settings->blueMultiplier, settings->blueMultiplierOscillator);
  }
};

#endif
