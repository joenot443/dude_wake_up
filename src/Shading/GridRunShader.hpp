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
  std::shared_ptr<Parameter> gridSkew;
  std::shared_ptr<Parameter> lineWobble;
  std::shared_ptr<Parameter> fogPosition;
  
  std::shared_ptr<WaveformOscillator> zoomOscillator;
  std::shared_ptr<WaveformOscillator> perspectiveOscillator;
  std::shared_ptr<WaveformOscillator> gridSkewOscillator;
  std::shared_ptr<WaveformOscillator> lineWobbleOscillator;
  std::shared_ptr<WaveformOscillator> fogPositionOscillator;
  
  GridRunSettings(std::string shaderId, json j, std::string name) :
  zoom(std::make_shared<Parameter>("Zoom", 0.1, -1.0, 1.0)),
  color(std::make_shared<Parameter>("Color", 0.0, -1.0, 1.0, ParameterType_Color)),
  perspective(std::make_shared<Parameter>("Perspective", 0.75, -5.0, 5.0)),
  gridSkew(std::make_shared<Parameter>("Grid Skew", 1.0, 0.0, 5.0)),
  lineWobble(std::make_shared<Parameter>("Line Wobble", 0.0, 0.0, 0.1)),
  fogPosition(std::make_shared<Parameter>("Fog Position", 0.2, -1.0, 1.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  perspectiveOscillator(std::make_shared<WaveformOscillator>(perspective)),
  gridSkewOscillator(std::make_shared<WaveformOscillator>(gridSkew)),
  lineWobbleOscillator(std::make_shared<WaveformOscillator>(lineWobble)),
  fogPositionOscillator(std::make_shared<WaveformOscillator>(fogPosition)),
  ShaderSettings(shaderId, j, name) {
    color->color = std::make_shared<std::array<float, 4>>(std::array<float, 4>({1.0f, 0.5f, 1.0f, 1.0f}));
    parameters = { zoom, perspective, color, gridSkew, lineWobble, fogPosition };
    oscillators = { zoomOscillator, perspectiveOscillator, gridSkewOscillator, lineWobbleOscillator, fogPositionOscillator };
    load(j);
    registerParameters();
  };
};

struct GridRunShader: Shader {
  GridRunSettings *settings;
  GridRunShader(GridRunSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Cyber Run");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform3f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2]);
    shader.setUniform1f("perspective", settings->perspective->value);
    shader.setUniform1f("gridSkew", settings->gridSkew->value);
    shader.setUniform1f("lineWobble", settings->lineWobble->value);
    shader.setUniform1f("fogPosition", settings->fogPosition->value);
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
    
    
    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderParameter(settings->perspective, settings->perspectiveOscillator);
    CommonViews::ShaderParameter(settings->gridSkew, settings->gridSkewOscillator);
    CommonViews::ShaderParameter(settings->lineWobble, settings->lineWobbleOscillator);
    CommonViews::ShaderParameter(settings->fogPosition, settings->fogPositionOscillator);
  }
};

#endif
