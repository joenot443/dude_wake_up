//
//  CompanionsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CompanionsShader_hpp
#define CompanionsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CompanionsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> positionX;
  std::shared_ptr<Parameter> positionY;
  std::shared_ptr<WaveformOscillator> positionXOscillator;
  std::shared_ptr<WaveformOscillator> positionYOscillator;

  std::shared_ptr<Parameter> flowSpeed;
  std::shared_ptr<WaveformOscillator> flowSpeedOscillator;

  std::shared_ptr<Parameter> zoom;
  std::shared_ptr<WaveformOscillator> zoomOscillator;

  std::shared_ptr<Parameter> octaves;

  std::shared_ptr<Parameter> lacunarity;
  std::shared_ptr<WaveformOscillator> lacunarityOscillator;

  std::shared_ptr<Parameter> gain;
  std::shared_ptr<WaveformOscillator> gainOscillator;

  std::shared_ptr<Parameter> focusPull;
  std::shared_ptr<WaveformOscillator> focusPullOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  CompanionsSettings(std::string shaderId, json j) :
  positionX(std::make_shared<Parameter>("Position X", 0.5, 0.0, 1.0)),
  positionY(std::make_shared<Parameter>("Position Y", 0.5, 0.0, 1.0)),
  positionXOscillator(std::make_shared<WaveformOscillator>(positionX)),
  positionYOscillator(std::make_shared<WaveformOscillator>(positionY)),
  flowSpeed(std::make_shared<Parameter>("Flow Speed", 1.0, 0.0, 3.0)),
  flowSpeedOscillator(std::make_shared<WaveformOscillator>(flowSpeed)),
  zoom(std::make_shared<Parameter>("Zoom", 1.0, 0.3, 3.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  octaves(std::make_shared<Parameter>("Octaves", 6.0, 1.0, 8.0, ParameterType_Int)),
  lacunarity(std::make_shared<Parameter>("Lacunarity", 2.0, 1.5, 3.0)),
  lacunarityOscillator(std::make_shared<WaveformOscillator>(lacunarity)),
  gain(std::make_shared<Parameter>("Gain", 0.2, 0.1, 0.7)),
  gainOscillator(std::make_shared<WaveformOscillator>(gain)),
  focusPull(std::make_shared<Parameter>("Focus Pull", 1.0, 0.0, 2.0)),
  focusPullOscillator(std::make_shared<WaveformOscillator>(focusPull)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  ShaderSettings(shaderId, j, "Companions") {
    parameters = { positionX, positionY, flowSpeed, zoom, octaves,
                   lacunarity, gain, focusPull, brightness, hueShift };
    oscillators = { positionXOscillator, positionYOscillator, flowSpeedOscillator,
                    zoomOscillator, lacunarityOscillator, gainOscillator,
                    focusPullOscillator, brightnessOscillator, hueShiftOscillator };
    load(j);
    registerParameters();
  };
};

struct CompanionsShader: Shader {
  CompanionsSettings *settings;
  CompanionsShader(CompanionsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Companions");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("position", settings->positionX->value, settings->positionY->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("flowSpeed", settings->flowSpeed->value);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform1i("octaves", (int)settings->octaves->value);
    shader.setUniform1f("lacunarity", settings->lacunarity->value);
    shader.setUniform1f("gain", settings->gain->value);
    shader.setUniform1f("focusPull", settings->focusPull->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
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
    return ShaderTypeCompanions;
  }

  void drawSettings() override {
    CommonViews::H3Title("Companions");
    CommonViews::MultiSlider("Position", formatString("##position%s", shaderId.c_str()), settings->positionX, settings->positionY, settings->positionXOscillator, settings->positionYOscillator, 0.5625);
    CommonViews::ShaderParameter(settings->flowSpeed, settings->flowSpeedOscillator);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderIntParameter(settings->octaves);
    CommonViews::ShaderParameter(settings->lacunarity, settings->lacunarityOscillator);
    CommonViews::ShaderParameter(settings->gain, settings->gainOscillator);
    CommonViews::ShaderParameter(settings->focusPull, settings->focusPullOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
  }
};

#endif /* CompanionsShader_hpp */
