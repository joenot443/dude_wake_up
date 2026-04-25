//
//  CyberWarehouseShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/8/26.
//

#ifndef CyberWarehouseShader_h
#define CyberWarehouseShader_h

#include <stdio.h>
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"
#include "Oscillator.hpp"

struct CyberWarehouseSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> zoom;
  std::shared_ptr<Parameter> colorR;
  std::shared_ptr<Parameter> colorG;
  std::shared_ptr<Parameter> colorB;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> zoomOscillator;
  std::shared_ptr<Oscillator> colorROscillator;
  std::shared_ptr<Oscillator> colorGOscillator;
  std::shared_ptr<Oscillator> colorBOscillator;

  CyberWarehouseSettings(std::string shaderId, json j, std::string name = "CyberWarehouse") :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 3.0)),
  zoom(std::make_shared<Parameter>("Zoom", 1.0, 0.1, 5.0)),
  colorR(std::make_shared<Parameter>("Color R", 0.26, 0.0, 1.0)),
  colorG(std::make_shared<Parameter>("Color G", 0.4, 0.0, 1.0)),
  colorB(std::make_shared<Parameter>("Color B", 0.6, 0.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  colorROscillator(std::make_shared<WaveformOscillator>(colorR)),
  colorGOscillator(std::make_shared<WaveformOscillator>(colorG)),
  colorBOscillator(std::make_shared<WaveformOscillator>(colorB)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, zoom, colorR, colorG, colorB};
    oscillators = {speedOscillator, zoomOscillator, colorROscillator, colorGOscillator, colorBOscillator};
    load(j);
    registerParameters();
  };
};

class CyberWarehouseShader : public Shader {
public:

  CyberWarehouseSettings *settings;
  CyberWarehouseShader(CyberWarehouseSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/CyberWarehouse");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 255);
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform1f("colorR", settings->colorR->value);
    shader.setUniform1f("colorG", settings->colorG->value);
    shader.setUniform1f("colorB", settings->colorB->value);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  int inputCount() override {
    return 0;
  }

  ShaderType type() override {
    return ShaderType::ShaderTypeCyberWarehouse;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderParameter(settings->colorR, settings->colorROscillator);
    CommonViews::ShaderParameter(settings->colorG, settings->colorGOscillator);
    CommonViews::ShaderParameter(settings->colorB, settings->colorBOscillator);
  }
};

#endif
