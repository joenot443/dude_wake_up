//
//  CrystalKaleidoscopeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/7/26.
//

#ifndef CrystalKaleidoscopeShader_h
#define CrystalKaleidoscopeShader_h

#include <stdio.h>
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"
#include "Oscillator.hpp"

struct CrystalKaleidoscopeSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> sides;
  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<Parameter> shift;

  std::shared_ptr<Oscillator> scaleOscillator;
  std::shared_ptr<Oscillator> sidesOscillator;
  std::shared_ptr<Oscillator> rotationOscillator;
  std::shared_ptr<Oscillator> shiftOscillator;

  CrystalKaleidoscopeSettings(std::string shaderId, json j, std::string name = "CrystalKaleidoscope") :
  scale(std::make_shared<Parameter>("Scale", 3.0, 1.0, 10.0)),
  sides(std::make_shared<Parameter>("Sides", 3.0, 3.0, 12.0, ParameterType_Int)),
  rotation(std::make_shared<Parameter>("Rotation", 0.0, 0.0, TWO_PI)),
  shift(std::make_shared<Parameter>("Shift", 0.0, -1.0, 1.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  sidesOscillator(std::make_shared<WaveformOscillator>(sides)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  shiftOscillator(std::make_shared<WaveformOscillator>(shift)),
  ShaderSettings(shaderId, j, name) {
    parameters = {scale, sides, rotation, shift};
    oscillators = {scaleOscillator, sidesOscillator, rotationOscillator, shiftOscillator};
    load(j);
    registerParameters();
  };
};

class CrystalKaleidoscopeShader : public Shader {
public:

  CrystalKaleidoscopeSettings *settings;
  CrystalKaleidoscopeShader(CrystalKaleidoscopeSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/CrystalKaleidoscope");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    frame->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("sides", settings->sides->value);
    shader.setUniform1f("rotation", settings->rotation->value);
    shader.setUniform1f("shift", settings->shift->value);
    shader.setUniformTexture("tex", *frame, 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderType::ShaderTypeCrystalKaleidoscope;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderIntParameter(settings->sides);
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
    CommonViews::ShaderParameter(settings->shift, settings->shiftOscillator);
  }
};

#endif
