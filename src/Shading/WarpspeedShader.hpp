//
//  WarpspeedShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WarpspeedShader_hpp
#define WarpspeedShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WarpspeedSettings: public ShaderSettings {
  // Motion
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<Parameter> twist;
  std::shared_ptr<WaveformOscillator> twistOscillator;

  // Tunnel geometry
  std::shared_ptr<Parameter> tunnelDepth;
  std::shared_ptr<WaveformOscillator> tunnelDepthOscillator;
  std::shared_ptr<Parameter> noiseScale;
  std::shared_ptr<WaveformOscillator> noiseScaleOscillator;

  // Look
  std::shared_ptr<Parameter> contrast;
  std::shared_ptr<WaveformOscillator> contrastOscillator;
  std::shared_ptr<Parameter> centerGlow;
  std::shared_ptr<WaveformOscillator> centerGlowOscillator;
  std::shared_ptr<Parameter> brightness;
  std::shared_ptr<WaveformOscillator> brightnessOscillator;

  // Color
  std::shared_ptr<Parameter> bgColor;
  std::shared_ptr<Parameter> starColor;

  WarpspeedSettings(std::string shaderId, json j) :
  // Motion
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 5.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  twist(std::make_shared<Parameter>("Twist", 0.0, -2.0, 2.0)),
  twistOscillator(std::make_shared<WaveformOscillator>(twist)),

  // Tunnel geometry
  tunnelDepth(std::make_shared<Parameter>("Tunnel Depth", 0.005, 0.0005, 0.05)),
  tunnelDepthOscillator(std::make_shared<WaveformOscillator>(tunnelDepth)),
  noiseScale(std::make_shared<Parameter>("Noise Scale", 10.0, 2.0, 30.0)),
  noiseScaleOscillator(std::make_shared<WaveformOscillator>(noiseScale)),

  // Look
  contrast(std::make_shared<Parameter>("Contrast", 10.0, 1.0, 20.0)),
  contrastOscillator(std::make_shared<WaveformOscillator>(contrast)),
  centerGlow(std::make_shared<Parameter>("Center Glow", 1.0, 0.0, 3.0)),
  centerGlowOscillator(std::make_shared<WaveformOscillator>(centerGlow)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.0, 3.0)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),

  // Color
  bgColor(std::make_shared<Parameter>("Background Color", ParameterType_Color)),
  starColor(std::make_shared<Parameter>("Star Color", ParameterType_Color)),

  ShaderSettings(shaderId, j, "Warpspeed") {
    bgColor->setColor({0.0, 0.0, 0.0, 1.0});
    starColor->setColor({1.0, 1.0, 1.0, 1.0});
    parameters = {
      speed, twist,
      tunnelDepth, noiseScale,
      contrast, centerGlow, brightness,
      bgColor, starColor
    };
    oscillators = {
      speedOscillator, twistOscillator,
      tunnelDepthOscillator, noiseScaleOscillator,
      contrastOscillator, centerGlowOscillator, brightnessOscillator
    };
    load(j);
    registerParameters();
  };
};

struct WarpspeedShader: Shader {
  WarpspeedSettings *settings;
  WarpspeedShader(WarpspeedSettings *settings) : settings(settings), Shader(settings) {};
  void setup() override {
    shader.load("shaders/Warpspeed");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    // Motion
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("twist", settings->twist->value);

    // Tunnel geometry
    shader.setUniform1f("tunnelDepth", settings->tunnelDepth->value);
    shader.setUniform1f("noiseScale", settings->noiseScale->value);

    // Look
    shader.setUniform1f("contrast", settings->contrast->value);
    shader.setUniform1f("centerGlow", settings->centerGlow->value);
    shader.setUniform1f("brightness", settings->brightness->value);

    // Color
    shader.setUniform3f("bgColor",
                       settings->bgColor->color->data()[0],
                       settings->bgColor->color->data()[1],
                       settings->bgColor->color->data()[2]);
    shader.setUniform3f("starColor",
                       settings->starColor->color->data()[0],
                       settings->starColor->color->data()[1],
                       settings->starColor->color->data()[2]);

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
    return ShaderTypeWarpspeed;
  }

  void drawSettings() override {
    CommonViews::H4Title("Motion");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->twist, settings->twistOscillator);

    CommonViews::H4Title("Tunnel");
    CommonViews::ShaderParameter(settings->tunnelDepth, settings->tunnelDepthOscillator);
    CommonViews::ShaderParameter(settings->noiseScale, settings->noiseScaleOscillator);

    CommonViews::H4Title("Look");
    CommonViews::ShaderParameter(settings->contrast, settings->contrastOscillator);
    CommonViews::ShaderParameter(settings->centerGlow, settings->centerGlowOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);

    CommonViews::H4Title("Color");
    CommonViews::ShaderColor(settings->bgColor);
    CommonViews::ShaderColor(settings->starColor);
  }
};

#endif
