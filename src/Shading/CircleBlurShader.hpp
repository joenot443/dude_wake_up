//
//  CircleBlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CircleBlurShader_hpp
#define CircleBlurShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CircleBlurSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> blurRadius;
  std::shared_ptr<WaveformOscillator> blurRadiusOscillator;

  std::shared_ptr<Parameter> channelSeparation;
  std::shared_ptr<WaveformOscillator> channelSeparationOscillator;

  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<WaveformOscillator> thresholdOscillator;

  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<WaveformOscillator> rotationOscillator;

  std::shared_ptr<Parameter> spiral;
  std::shared_ptr<WaveformOscillator> spiralOscillator;

  std::shared_ptr<Parameter> hueShift;
  std::shared_ptr<WaveformOscillator> hueShiftOscillator;

  std::shared_ptr<Parameter> chromaticStrength;
  std::shared_ptr<WaveformOscillator> chromaticStrengthOscillator;

  CircleBlurSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  blurRadius(std::make_shared<Parameter>("Blur Radius", 0.05, 0.0, 0.2)),
  blurRadiusOscillator(std::make_shared<WaveformOscillator>(blurRadius)),
  channelSeparation(std::make_shared<Parameter>("Channel Separation", 1.0, 0.0, 1.0)),
  channelSeparationOscillator(std::make_shared<WaveformOscillator>(channelSeparation)),
  threshold(std::make_shared<Parameter>("Threshold", 0.35, 0.0, 1.0)),
  thresholdOscillator(std::make_shared<WaveformOscillator>(threshold)),
  rotation(std::make_shared<Parameter>("Rotation", 0.0, 0.0, 6.28)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  spiral(std::make_shared<Parameter>("Spiral", 0.0, 0.0, 1.0)),
  spiralOscillator(std::make_shared<WaveformOscillator>(spiral)),
  hueShift(std::make_shared<Parameter>("Hue Shift", 0.0, 0.0, 1.0)),
  hueShiftOscillator(std::make_shared<WaveformOscillator>(hueShift)),
  chromaticStrength(std::make_shared<Parameter>("Chromatic Strength", 1.0, 0.0, 1.0)),
  chromaticStrengthOscillator(std::make_shared<WaveformOscillator>(chromaticStrength)),
  ShaderSettings(shaderId, j, "CircleBlur") {
    parameters = { shaderValue, blurRadius, channelSeparation, threshold, rotation, spiral, hueShift, chromaticStrength };
    oscillators = { shaderValueOscillator, blurRadiusOscillator, channelSeparationOscillator, thresholdOscillator, rotationOscillator, spiralOscillator, hueShiftOscillator, chromaticStrengthOscillator };
    load(j);
    registerParameters();
  };
};

struct CircleBlurShader: Shader {
  CircleBlurSettings *settings;
  CircleBlurShader(CircleBlurSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/CircleBlur");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    // Clear canvas to ensure alpha channel is properly set
    ofClear(0, 0, 0, 0);
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("blurRadius", settings->blurRadius->value);
    shader.setUniform1f("channelSeparation", settings->channelSeparation->value);
    shader.setUniform1f("threshold", settings->threshold->value);
    shader.setUniform1f("rotation", settings->rotation->value);
    shader.setUniform1f("spiral", settings->spiral->value);
    shader.setUniform1f("hueShift", settings->hueShift->value);
    shader.setUniform1f("chromaticStrength", settings->chromaticStrength->value);
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
    return ShaderTypeCircleBlur;
  }

  void drawSettings() override {
    CommonViews::H3Title("CircleBlur");

    CommonViews::ShaderParameter(settings->blurRadius, settings->blurRadiusOscillator);
    CommonViews::ShaderParameter(settings->channelSeparation, settings->channelSeparationOscillator);
    CommonViews::ShaderParameter(settings->threshold, settings->thresholdOscillator);
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
    CommonViews::ShaderParameter(settings->spiral, settings->spiralOscillator);
    CommonViews::ShaderParameter(settings->hueShift, settings->hueShiftOscillator);
    CommonViews::ShaderParameter(settings->chromaticStrength, settings->chromaticStrengthOscillator);
  }
};

#endif /* CircleBlurShader_hpp */
