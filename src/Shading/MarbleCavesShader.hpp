//
//  MarbleCavesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MarbleCavesShader_hpp
#define MarbleCavesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include "AudioSourceService.hpp"
#include <stdio.h>

struct MarbleCavesSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  std::shared_ptr<Parameter> twist;
  std::shared_ptr<WaveformOscillator> twistOscillator;

  std::shared_ptr<Parameter> offset;
  std::shared_ptr<WaveformOscillator> offsetOscillator;

  std::shared_ptr<Parameter> steps;

  std::shared_ptr<Parameter> detailA;
  std::shared_ptr<WaveformOscillator> detailAOscillator;

  std::shared_ptr<Parameter> detailB;
  std::shared_ptr<WaveformOscillator> detailBOscillator;

  std::shared_ptr<Parameter> density;
  std::shared_ptr<WaveformOscillator> densityOscillator;

  std::shared_ptr<Parameter> translucency;
  std::shared_ptr<WaveformOscillator> translucencyOscillator;

  std::shared_ptr<Parameter> reflection;
  std::shared_ptr<WaveformOscillator> reflectionOscillator;

  std::shared_ptr<Parameter> poolHeight;
  std::shared_ptr<WaveformOscillator> poolHeightOscillator;

  std::shared_ptr<Parameter> colorSpeed;
  std::shared_ptr<WaveformOscillator> colorSpeedOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> wisp;
  std::shared_ptr<WaveformOscillator> wispOscillator;

  std::shared_ptr<Parameter> audioAmount;
  std::shared_ptr<WaveformOscillator> audioAmountOscillator;

  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  MarbleCavesSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 4.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  twist(std::make_shared<Parameter>("Twist", 1.0, 0.5, 1.5)),
  twistOscillator(std::make_shared<WaveformOscillator>(twist)),
  offset(std::make_shared<Parameter>("Offset", 0.0, -10.0, 10.0)),
  offsetOscillator(std::make_shared<WaveformOscillator>(offset)),
  steps(std::make_shared<Parameter>("Steps", 79.0, 30.0, 120.0, ParameterType_Int)),
  detailA(std::make_shared<Parameter>("Detail A", 8.0, 2.0, 20.0)),
  detailAOscillator(std::make_shared<WaveformOscillator>(detailA)),
  detailB(std::make_shared<Parameter>("Detail B", 24.0, 8.0, 48.0)),
  detailBOscillator(std::make_shared<WaveformOscillator>(detailB)),
  density(std::make_shared<Parameter>("Density", 4.0, 1.0, 12.0)),
  densityOscillator(std::make_shared<WaveformOscillator>(density)),
  translucency(std::make_shared<Parameter>("Translucency", 0.0005, 0.0001, 0.005)),
  translucencyOscillator(std::make_shared<WaveformOscillator>(translucency)),
  reflection(std::make_shared<Parameter>("Reflection", 0.1, 0.0, 1.0)),
  reflectionOscillator(std::make_shared<WaveformOscillator>(reflection)),
  poolHeight(std::make_shared<Parameter>("Pool Height", 0.1, -0.5, 0.5)),
  poolHeightOscillator(std::make_shared<WaveformOscillator>(poolHeight)),
  colorSpeed(std::make_shared<Parameter>("Color Speed", 5.0, 0.0, 15.0)),
  colorSpeedOscillator(std::make_shared<WaveformOscillator>(colorSpeed)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 6.2832)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  wisp(std::make_shared<Parameter>("Wisp", 1.0, 0.0, 3.0)),
  wispOscillator(std::make_shared<WaveformOscillator>(wisp)),
  audioAmount(std::make_shared<Parameter>("Audio Amount", 2.0, 0.0, 8.0)),
  audioAmountOscillator(std::make_shared<WaveformOscillator>(audioAmount)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.1, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, "MarbleCaves") {
    parameters = { speed, twist, offset, steps, detailA, detailB, density, translucency,
                   reflection, poolHeight, colorSpeed, hueShift, wisp, audioAmount, brightness };
    oscillators = { speedOscillator, twistOscillator, offsetOscillator, detailAOscillator,
                    detailBOscillator, densityOscillator, translucencyOscillator,
                    reflectionOscillator, poolHeightOscillator, colorSpeedOscillator,
                    hueShiftOscillator, wispOscillator, audioAmountOscillator, brightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct MarbleCavesShader: Shader {
  MarbleCavesSettings *settings;
  MarbleCavesShader(MarbleCavesSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/MarbleCaves");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1f("audioAmount", settings->audioAmount->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("twist", settings->twist->value);
    shader.setUniform1f("offset", settings->offset->value);
    shader.setUniform1f("steps", settings->steps->value);
    shader.setUniform1f("detailA", settings->detailA->value);
    shader.setUniform1f("detailB", settings->detailB->value);
    shader.setUniform1f("density", settings->density->value);
    shader.setUniform1f("translucency", settings->translucency->value);
    shader.setUniform1f("reflection", settings->reflection->value);
    shader.setUniform1f("poolHeight", settings->poolHeight->value);
    shader.setUniform1f("colorSpeed", settings->colorSpeed->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("wisp", settings->wisp->value);
    shader.setUniform1f("brightness", settings->brightness->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
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
    return ShaderTypeMarbleCaves;
  }

  void drawSettings() override {
    CommonViews::H3Title("MarbleCaves");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->twist, settings->twistOscillator);
    CommonViews::ShaderParameter(settings->offset, settings->offsetOscillator);
    CommonViews::ShaderIntParameter(settings->steps);
    CommonViews::ShaderParameter(settings->detailA, settings->detailAOscillator);
    CommonViews::ShaderParameter(settings->detailB, settings->detailBOscillator);
    CommonViews::ShaderParameter(settings->density, settings->densityOscillator);
    CommonViews::ShaderParameter(settings->translucency, settings->translucencyOscillator);
    CommonViews::ShaderParameter(settings->reflection, settings->reflectionOscillator);
    CommonViews::ShaderParameter(settings->poolHeight, settings->poolHeightOscillator);
    CommonViews::ShaderParameter(settings->colorSpeed, settings->colorSpeedOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->wisp, settings->wispOscillator);
    CommonViews::ShaderParameter(settings->audioAmount, settings->audioAmountOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* MarbleCavesShader_hpp */
