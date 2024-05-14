//
//  CannyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CannyShader_hpp
#define CannyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CannySettings: public ShaderSettings {
  std::shared_ptr<Parameter> backgroundColor;
  std::shared_ptr<Parameter> alpha;

  CannySettings(std::string shaderId, json j, std::string name) :
  backgroundColor(std::make_shared<Parameter>("Background Color", 1.0, -1.0, 2.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  ShaderSettings(shaderId, j, name) {
    parameters = { backgroundColor, alpha };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct CannyShader: Shader {
  CannySettings *settings;
  CannyShader(CannySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Canny");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform4f("backgroundColor", settings->backgroundColor->color->data()[0], settings->backgroundColor->color->data()[1], settings->backgroundColor->color->data()[2], settings->alpha->value);
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
    return ShaderTypeCanny;
  }

  void drawSettings() override {
    CommonViews::H3Title("Canny");

    CommonViews::ShaderColor(settings->backgroundColor);
    CommonViews::ShaderParameter(settings->alpha, nullptr);
  }
};

#endif /* CannyShader_hpp */
