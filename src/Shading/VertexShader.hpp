//
//  VertexShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef VertexShader_hpp
#define VertexShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct VertexSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;

  VertexSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("color", 1.0  , -1.0, 2.0, ParameterType_Color)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { color };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct VertexShader: Shader {
  VertexSettings *settings;
  VertexShader(VertexSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Vertex");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
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
    return ShaderTypeVertex;
  }

  void drawSettings() override {
    CommonViews::H3Title("Vertex");

    CommonViews::ShaderColor(settings->color);
  }
};

#endif /* VertexShader_hpp */
