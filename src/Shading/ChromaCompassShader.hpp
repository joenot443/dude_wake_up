//
//  ChromaCompassShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ChromaCompassShader_hpp
#define ChromaCompassShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ChromaCompassSettings: public ShaderSettings {
  std::shared_ptr<Parameter> rotationAngle;
  std::shared_ptr<WaveformOscillator> rotationAngleOscillator;

  std::shared_ptr<Parameter> pivotU;
  std::shared_ptr<WaveformOscillator> pivotUOscillator;

  std::shared_ptr<Parameter> pivotV;
  std::shared_ptr<WaveformOscillator> pivotVOscillator;

  std::shared_ptr<Parameter> saturationGamma;
  std::shared_ptr<WaveformOscillator> saturationGammaOscillator;

  std::shared_ptr<Parameter> lumaShift;
  std::shared_ptr<WaveformOscillator> lumaShiftOscillator;

  std::shared_ptr<Parameter> mixAmount;
  std::shared_ptr<WaveformOscillator> mixAmountOscillator;

  ChromaCompassSettings(std::string shaderId, json j) :
  rotationAngle(std::make_shared<Parameter>("Rotation Angle", 0.0, -6.2832, 6.2832)),
  rotationAngleOscillator(std::make_shared<WaveformOscillator>(rotationAngle)),
  pivotU(std::make_shared<Parameter>("Pivot U", 0.0, -0.5, 0.5)),
  pivotUOscillator(std::make_shared<WaveformOscillator>(pivotU)),
  pivotV(std::make_shared<Parameter>("Pivot V", 0.0, -0.5, 0.5)),
  pivotVOscillator(std::make_shared<WaveformOscillator>(pivotV)),
  saturationGamma(std::make_shared<Parameter>("Saturation Gamma", 1.0, 0.25, 4.0)),
  saturationGammaOscillator(std::make_shared<WaveformOscillator>(saturationGamma)),
  lumaShift(std::make_shared<Parameter>("Luma Shift", 0.0, -0.25, 0.25)),
  lumaShiftOscillator(std::make_shared<WaveformOscillator>(lumaShift)),
  mixAmount(std::make_shared<Parameter>("Mix", 1.0, 0.0, 1.0)),
  mixAmountOscillator(std::make_shared<WaveformOscillator>(mixAmount)),
  ShaderSettings(shaderId, j, "ChromaCompass") {
    parameters = { rotationAngle, pivotU, pivotV, saturationGamma, lumaShift, mixAmount };
    oscillators = { rotationAngleOscillator, pivotUOscillator, pivotVOscillator, saturationGammaOscillator, lumaShiftOscillator, mixAmountOscillator };
    load(j);
    registerParameters();
  };
};

struct ChromaCompassShader: Shader {
  ChromaCompassSettings *settings;
  ChromaCompassShader(ChromaCompassSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/ChromaCompass");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("rotationAngle", settings->rotationAngle->value);
    shader.setUniform1f("pivotU", settings->pivotU->value);
    shader.setUniform1f("pivotV", settings->pivotV->value);
    shader.setUniform1f("saturationGamma", settings->saturationGamma->value);
    shader.setUniform1f("lumaShift", settings->lumaShift->value);
    shader.setUniform1f("mixAmount", settings->mixAmount->value);
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
    return ShaderTypeChromaCompass;
  }

  void drawSettings() override {
    CommonViews::H3Title("ChromaCompass");

    CommonViews::ShaderParameter(settings->rotationAngle, settings->rotationAngleOscillator);
    CommonViews::ShaderParameter(settings->pivotU, settings->pivotUOscillator);
    CommonViews::ShaderParameter(settings->pivotV, settings->pivotVOscillator);
    CommonViews::ShaderParameter(settings->saturationGamma, settings->saturationGammaOscillator);
    CommonViews::ShaderParameter(settings->lumaShift, settings->lumaShiftOscillator);
    CommonViews::ShaderParameter(settings->mixAmount, settings->mixAmountOscillator);
  }
};

#endif /* ChromaCompassShader_hpp */
