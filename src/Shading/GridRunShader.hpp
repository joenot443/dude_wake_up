//
//  GridRunShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GridRunShader_hpp
#define GridRunShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GridRunSettings: public ShaderSettings {
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> zoom;
  std::shared_ptr<Parameter> perspective;
  std::shared_ptr<WaveformOscillator> zoomOscillator;
  std::shared_ptr<WaveformOscillator> perspectiveOscillator;
  
  GridRunSettings(std::string shaderId, json j, std::string name) :
  zoom(std::make_shared<Parameter>("Zoom", 0.1, -1.0, 1.0)),
  color(std::make_shared<Parameter>("Color", 0.0, -1.0, 1.0, ParameterType_Color)),
  perspective(std::make_shared<Parameter>("Perspective", 0.75, -5.0, 5.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  perspectiveOscillator(std::make_shared<WaveformOscillator>(perspective)),
  ShaderSettings(shaderId, j, name) {
    color->color = std::make_shared<std::array<float, 4>>(std::array<float, 4>({1.0f, 0.5f, 1.0f, 1.0f}));
    parameters = { zoom, perspective, color };
    oscillators = { zoomOscillator, perspectiveOscillator };
    load(j);
    registerParameters();
  };
};

struct GridRunShader: Shader {
  GridRunSettings *settings;
  GridRunShader(GridRunSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/GridRun");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform1f("perspective", settings->perspective->value);
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
    return ShaderTypeGridRun;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("GridRun");
    
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderParameter(settings->perspective, settings->perspectiveOscillator);
  }
};

#endif /* GridRunShader_hpp */
