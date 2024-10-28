//
//  OneBitDitherShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OneBitDitherShader_hpp
#define OneBitDitherShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OneBitDitherSettings: public ShaderSettings {
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  
  std::shared_ptr<Parameter> lightColor;
  std::shared_ptr<Parameter> darkColor;

  OneBitDitherSettings(std::string shaderId, json j) :
  lightColor(std::make_shared<Parameter>("Light Color", 1.0, -1.0, 2.0, ParameterType_Color)),
  darkColor(std::make_shared<Parameter>("Dark Color", 1.0, -1.0, 2.0, ParameterType_Color)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  ShaderSettings(shaderId, j, "OneBitDither") {
    lightColor->setColor({0.5, 0.8, 0.65, 1.0});
    darkColor->setColor({0.2, 0.3, 0.4, 1.0});
    parameters = { scale, lightColor, darkColor };
    audioReactiveParameter = scale;
    load(j);
    registerParameters();
  };
};

struct OneBitDitherShader: Shader {
  OneBitDitherSettings *settings;
  OneBitDitherShader(OneBitDitherSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/OneBitDither");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform3f("lightColor", settings->lightColor->color->data()[0], settings->lightColor->color->data()[1], settings->lightColor->color->data()[2]);
    shader.setUniform3f("darkColor", settings->darkColor->color->data()[0], settings->darkColor->color->data()[1], settings->darkColor->color->data()[2]);
    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypeOneBitDither;
  }

  void drawSettings() override {
    CommonViews::H3Title("OneBitDither");

    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderColor(settings->lightColor);
    CommonViews::ShaderColor(settings->darkColor);
  }
};

#endif
