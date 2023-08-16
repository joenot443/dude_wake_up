//
//  SolidColorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SolidColorShader_hpp
#define SolidColorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SolidColorSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;

  SolidColorSettings(std::string shaderId, json j) :
  color(std::make_shared<Parameter>("Color", 1.0, -1.0, 2.0)),
  ShaderSettings(shaderId, j) {
    parameters = { color };
    load(j);
    registerParameters();
  };
};

struct SolidColorShader: Shader {
  SolidColorSettings *settings;
  SolidColorShader(SolidColorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/SolidColor");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], 1.0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeSolidColor;
  }

  void drawSettings() override {
    CommonViews::H3Title("SolidColor");

    CommonViews::ShaderColor(settings->color);
  }
};

#endif /* SolidColorShader_hpp */
