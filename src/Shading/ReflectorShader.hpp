//
//  ReflectorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ReflectorShader_hpp
#define ReflectorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ReflectorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> polyU;
  std::shared_ptr<Parameter> polyV;
  std::shared_ptr<Parameter> polyW;
  
  std::shared_ptr<WaveformOscillator> polyUOscillator;
  std::shared_ptr<WaveformOscillator> polyVOscillator;
  std::shared_ptr<WaveformOscillator> polyWOscillator;

  ReflectorSettings(std::string shaderId, json j) :
  polyU(std::make_shared<Parameter>("U", 0.0, 0.0, 1.0)),
  polyV(std::make_shared<Parameter>("V", 0.001, 0.001, 1.0)),
  polyW(std::make_shared<Parameter>("W", 0.0, 0.0, 1.0)),
  polyUOscillator(std::make_shared<WaveformOscillator>(polyU)),
  polyVOscillator(std::make_shared<WaveformOscillator>(polyV)),
  polyWOscillator(std::make_shared<WaveformOscillator>(polyW)),
  ShaderSettings(shaderId, j, "Reflector") {
    parameters = { polyU, polyV, polyW };
    oscillators = { polyUOscillator, polyVOscillator, polyWOscillator };
    load(j);
    registerParameters();
  };
};

struct ReflectorShader: Shader {
  ReflectorSettings *settings;
  ReflectorShader(ReflectorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Reflector");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("poly_u", settings->polyU->value);
    shader.setUniform1f("poly_v", settings->polyV->value);
    shader.setUniform1f("poly_w", settings->polyW->value);
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
    return ShaderTypeReflector;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->polyU, settings->polyUOscillator);
    CommonViews::ShaderParameter(settings->polyV, settings->polyVOscillator);
    CommonViews::ShaderParameter(settings->polyW, settings->polyWOscillator);
  }
};

#endif
