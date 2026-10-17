//
//  RetroPCShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RetroPCShader_hpp
#define RetroPCShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RetroPCSettings: public ShaderSettings {
  std::shared_ptr<Parameter> gamma;
  std::shared_ptr<WaveformOscillator> gammaOscillator;

  std::shared_ptr<Parameter> pixelSize;
  std::shared_ptr<WaveformOscillator> pixelSizeOscillator;

  std::shared_ptr<Parameter> ditherAmount;
  std::shared_ptr<WaveformOscillator> ditherAmountOscillator;

  std::shared_ptr<Parameter> palette;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<WaveformOscillator> saturationOscillator;

  RetroPCSettings(std::string shaderId, json j) :
  gamma(std::make_shared<Parameter>("Gamma", 1.0, 0.1, 3.0)),
  gammaOscillator(std::make_shared<WaveformOscillator>(gamma)),
  pixelSize(std::make_shared<Parameter>("Pixel Size", 4.0, 1.0, 16.0)),
  pixelSizeOscillator(std::make_shared<WaveformOscillator>(pixelSize)),
  ditherAmount(std::make_shared<Parameter>("Dither Amount", 1.0, 0.0, 1.0)),
  ditherAmountOscillator(std::make_shared<WaveformOscillator>(ditherAmount)),
  palette(std::make_shared<Parameter>("Palette", 0, 0, 7, ParameterType_Int)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, -1.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  saturation(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 2.0)),
  saturationOscillator(std::make_shared<WaveformOscillator>(saturation)),
  ShaderSettings(shaderId, j, "RetroPC") {
    palette->options = {"ZX Spectrum", "CGA 0 (Cyan/Magenta)", "CGA 1 (Green/Red)", "Game Boy", "Commodore 64", "Mono Green", "Mono Amber", "Mono White"};
    parameters = { gamma, pixelSize, ditherAmount, palette, hueShift, saturation };
    oscillators = { gammaOscillator, pixelSizeOscillator, ditherAmountOscillator, hueShiftOscillator, saturationOscillator };
    load(j);
    registerParameters();
  };
};

struct RetroPCShader: Shader {
  RetroPCSettings *settings;
  RetroPCShader(RetroPCSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/RetroPC");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("gamma", settings->gamma->value);
    shader.setUniform1f("pixelSize", settings->pixelSize->value);
    shader.setUniform1f("ditherAmount", settings->ditherAmount->value);
    shader.setUniform1i("palette", settings->palette->intValue);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("saturation", settings->saturation->value);
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
    return ShaderTypeRetroPC;
  }

  void drawSettings() override {
    CommonViews::H3Title("RetroPC");

    CommonViews::ShaderOption(settings->palette, settings->palette->options);
    CommonViews::ShaderParameter(settings->gamma, settings->gammaOscillator);
    CommonViews::ShaderParameter(settings->pixelSize, settings->pixelSizeOscillator);
    CommonViews::ShaderParameter(settings->ditherAmount, settings->ditherAmountOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
  }
};

#endif /* RetroPCShader_hpp */
