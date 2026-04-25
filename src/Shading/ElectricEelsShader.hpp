//
//  ElectricEelsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ElectricEelsShader_hpp
#define ElectricEelsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ElectricEelsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> orbX;
  std::shared_ptr<WaveformOscillator> orbXOscillator;

  std::shared_ptr<Parameter> orbY;
  std::shared_ptr<WaveformOscillator> orbYOscillator;

  std::shared_ptr<Parameter> flowSpeed;
  std::shared_ptr<WaveformOscillator> flowSpeedOscillator;

  std::shared_ptr<Parameter> twistSpeed;
  std::shared_ptr<WaveformOscillator> twistSpeedOscillator;

  std::shared_ptr<Parameter> cameraSway;
  std::shared_ptr<WaveformOscillator> cameraSwayOscillator;

  std::shared_ptr<Parameter> zoom;
  std::shared_ptr<WaveformOscillator> zoomOscillator;

  std::shared_ptr<Parameter> layerCount;

  std::shared_ptr<Parameter> repetition;

  std::shared_ptr<Parameter> glowIntensity;
  std::shared_ptr<WaveformOscillator> glowIntensityOscillator;

  std::shared_ptr<Parameter> thickness;
  std::shared_ptr<WaveformOscillator> thicknessOscillator;

  std::shared_ptr<Parameter> fogDecay;
  std::shared_ptr<WaveformOscillator> fogDecayOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  ElectricEelsSettings(std::string shaderId, json j) :
  orbX(std::make_shared<Parameter>("Orb X", 0.0, -2.0, 2.0)),
  orbXOscillator(std::make_shared<WaveformOscillator>(orbX)),
  orbY(std::make_shared<Parameter>("Orb Y", 0.0, -2.0, 2.0)),
  orbYOscillator(std::make_shared<WaveformOscillator>(orbY)),
  flowSpeed(std::make_shared<Parameter>("Flow Speed", 1.5, 0.1, 4.0)),
  flowSpeedOscillator(std::make_shared<WaveformOscillator>(flowSpeed)),
  twistSpeed(std::make_shared<Parameter>("Twist Speed", 0.1, 0.0, 2.0)),
  twistSpeedOscillator(std::make_shared<WaveformOscillator>(twistSpeed)),
  cameraSway(std::make_shared<Parameter>("Camera Sway", 0.2, 0.0, 1.0)),
  cameraSwayOscillator(std::make_shared<WaveformOscillator>(cameraSway)),
  zoom(std::make_shared<Parameter>("Zoom", 2.0, 0.5, 4.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  layerCount(std::make_shared<Parameter>("Layer Count", 4.0, 1.0, 8.0, ParameterType_Int)),
  repetition(std::make_shared<Parameter>("Repetition", 27.0, 6.0, 64.0, ParameterType_Int)),
  glowIntensity(std::make_shared<Parameter>("Glow Intensity", 1.0, 0.1, 5.0)),
  glowIntensityOscillator(std::make_shared<WaveformOscillator>(glowIntensity)),
  thickness(std::make_shared<Parameter>("Thickness", 1.0, 0.3, 3.0)),
  thicknessOscillator(std::make_shared<WaveformOscillator>(thickness)),
  fogDecay(std::make_shared<Parameter>("Fog Decay", 0.04, 0.0, 0.2)),
  fogDecayOscillator(std::make_shared<WaveformOscillator>(fogDecay)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  ShaderSettings(shaderId, j, "ElectricEels") {
    parameters = { orbX, orbY, flowSpeed, twistSpeed, cameraSway, zoom, layerCount, repetition, glowIntensity, thickness, fogDecay, hueShift };
    oscillators = { orbXOscillator, orbYOscillator, flowSpeedOscillator, twistSpeedOscillator, cameraSwayOscillator, zoomOscillator, glowIntensityOscillator, thicknessOscillator, fogDecayOscillator, hueShiftOscillator };
    load(j);
    registerParameters();
  };
};

struct ElectricEelsShader: Shader {
  ElectricEelsSettings *settings;
  ElectricEelsShader(ElectricEelsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/ElectricEels");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("orbX", settings->orbX->value);
    shader.setUniform1f("orbY", settings->orbY->value);
    shader.setUniform1f("flowSpeed", settings->flowSpeed->value);
    shader.setUniform1f("twistSpeed", settings->twistSpeed->value);
    shader.setUniform1f("cameraSway", settings->cameraSway->value);
    shader.setUniform1f("zoom", settings->zoom->value);
    shader.setUniform1i("layerCount", (int)settings->layerCount->value);
    shader.setUniform1i("repetition", (int)settings->repetition->value);
    shader.setUniform1f("glowIntensity", settings->glowIntensity->value);
    shader.setUniform1f("thickness", settings->thickness->value);
    shader.setUniform1f("fogDecay", settings->fogDecay->value);
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
    return ShaderTypeElectricEels;
  }

  void drawSettings() override {
    CommonViews::H3Title("ElectricEels");
    CommonViews::MultiSlider("Orb Position", settings->shaderId, settings->orbX, settings->orbY, settings->orbXOscillator, settings->orbYOscillator);
    CommonViews::ShaderParameter(settings->flowSpeed, settings->flowSpeedOscillator);
    CommonViews::ShaderParameter(settings->twistSpeed, settings->twistSpeedOscillator);
    CommonViews::ShaderParameter(settings->cameraSway, settings->cameraSwayOscillator);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
    CommonViews::ShaderIntParameter(settings->layerCount);
    CommonViews::ShaderIntParameter(settings->repetition);
    CommonViews::ShaderParameter(settings->glowIntensity, settings->glowIntensityOscillator);
    CommonViews::ShaderParameter(settings->thickness, settings->thicknessOscillator);
    CommonViews::ShaderParameter(settings->fogDecay, settings->fogDecayOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
  }
};

#endif /* ElectricEelsShader_hpp */
