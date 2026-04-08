//
//  GlassyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GlassyShader_hpp
#define GlassyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GlassySettings: public ShaderSettings {
  std::shared_ptr<Parameter> specularPower;
  std::shared_ptr<WaveformOscillator> specularPowerOscillator;

  std::shared_ptr<Parameter> reflectivity;
  std::shared_ptr<WaveformOscillator> reflectivityOscillator;

  GlassySettings(std::string shaderId, json j) :
  specularPower(std::make_shared<Parameter>("Specular Power", 32.0, 4.0, 128.0)),
  specularPowerOscillator(std::make_shared<WaveformOscillator>(specularPower)),
  reflectivity(std::make_shared<Parameter>("Reflectivity", 0.9, 0.0, 1.0)),
  reflectivityOscillator(std::make_shared<WaveformOscillator>(reflectivity)),
  ShaderSettings(shaderId, j, "Glassy") {
    parameters = { specularPower, reflectivity };
    oscillators = { specularPowerOscillator, reflectivityOscillator };
    load(j);
    registerParameters();
  };
};

struct GlassyShader: Shader {
  GlassySettings *settings;
  GlassyShader(GlassySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Glassy");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("specularPower", settings->specularPower->value);
    shader.setUniform1f("reflectivity", settings->reflectivity->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
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
    return ShaderTypeGlassy;
  }

  void drawSettings() override {
    CommonViews::H3Title("Glassy");
    CommonViews::ShaderParameter(settings->specularPower, settings->specularPowerOscillator);
    CommonViews::ShaderParameter(settings->reflectivity, settings->reflectivityOscillator);
  }
};

#endif /* GlassyShader_hpp */
