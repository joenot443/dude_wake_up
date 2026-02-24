//
//  RuttEtraShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RuttEtraShader_hpp
#define RuttEtraShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RuttEtraSettings: public ShaderSettings {
  std::shared_ptr<Parameter> lines;
  std::shared_ptr<WaveformOscillator> linesOscillator;

  std::shared_ptr<Parameter> lineWidth;
  std::shared_ptr<WaveformOscillator> lineWidthOscillator;

  std::shared_ptr<Parameter> extrusion;
  std::shared_ptr<WaveformOscillator> extrusionOscillator;

  std::shared_ptr<Parameter> yScale;
  std::shared_ptr<WaveformOscillator> yScaleOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> fillOpacity;
  std::shared_ptr<WaveformOscillator> fillOpacityOscillator;

  RuttEtraSettings(std::string shaderId, json j) :
  lines(std::make_shared<Parameter>("Lines", 42.6, 10.0, 200.0)),
  linesOscillator(std::make_shared<WaveformOscillator>(lines)),
  lineWidth(std::make_shared<Parameter>("Line Width", 3.62, 0.1, 5.0)),
  lineWidthOscillator(std::make_shared<WaveformOscillator>(lineWidth)),
  extrusion(std::make_shared<Parameter>("Extrusion", 0.02, 0.0, 1.0)),
  extrusionOscillator(std::make_shared<WaveformOscillator>(extrusion)),
  yScale(std::make_shared<Parameter>("Y Scale", 0.98, 0.1, 2.0)),
  yScaleOscillator(std::make_shared<WaveformOscillator>(yScale)),
  brightness(std::make_shared<Parameter>("Brightness", 1.5, 0.0, 5.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  fillOpacity(std::make_shared<Parameter>("Fill Opacity", 1.0, 0.0, 1.0)),
  fillOpacityOscillator(std::make_shared<WaveformOscillator>(fillOpacity)),
  ShaderSettings(shaderId, j, "RuttEtra") {
    parameters = { lines, lineWidth, extrusion, yScale, brightness, fillOpacity };
    oscillators = { linesOscillator, lineWidthOscillator, extrusionOscillator, yScaleOscillator, brightnessOscillator, fillOpacityOscillator };
    load(j);
    registerParameters();
  };
};

struct RuttEtraShader: Shader {
  RuttEtraSettings *settings;
  RuttEtraShader(RuttEtraSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/RuttEtra");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("lines", settings->lines->value);
    shader.setUniform1f("lineWidth", settings->lineWidth->value);
    shader.setUniform1f("extrusion", settings->extrusion->value);
    shader.setUniform1f("yScale", settings->yScale->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("fillOpacity", settings->fillOpacity->value);
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
    return ShaderTypeRuttEtra;
  }

  void drawSettings() override {
    CommonViews::H3Title("RuttEtra");
    CommonViews::ShaderParameter(settings->lines, settings->linesOscillator);
    CommonViews::ShaderParameter(settings->lineWidth, settings->lineWidthOscillator);
    CommonViews::ShaderParameter(settings->extrusion, settings->extrusionOscillator);
    CommonViews::ShaderParameter(settings->yScale, settings->yScaleOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
    CommonViews::ShaderParameter(settings->fillOpacity, settings->fillOpacityOscillator);
  }
};

#endif /* RuttEtraShader_hpp */
