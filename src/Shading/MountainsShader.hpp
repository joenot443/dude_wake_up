#ifndef MountainsShader_hpp
#define MountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MountainsSettings: public ShaderSettings {
  public:
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> alpha;

  MountainsSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("Color", 1.0, -1.0, 2.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  ShaderSettings(shaderId, j, name) {
    parameters = { color, alpha };
    load(j);
    registerParameters();
  };
};

class MountainsShader: public Shader {
public:

  MountainsSettings *settings;
  MountainsShader(MountainsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
#ifdef TESTING
    shader.load("shaders/Mountains");
#endif
#ifdef RELEASE
    shader.load("shaders/Mountains");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], settings->alpha->value);
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
    return ShaderTypeMountains;
  }

  void drawSettings() override {
    CommonViews::H3Title("Mountains");
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->alpha, nullptr);
  }
};

#endif /* MountainsShader_hpp */
