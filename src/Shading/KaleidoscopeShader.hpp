//
//  KaleidoscopeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/27/22.
//

#ifndef KaleidoscopeShader_h
#define KaleidoscopeShader_h

#include <stdio.h>
#include "Shader.hpp"
#include "Parameter.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"
#include "Oscillator.hpp"

struct KaleidoscopeSettings : public ShaderSettings {
  std::shared_ptr<Parameter> sides;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> rotation;

  std::shared_ptr<Oscillator> sidesOscillator;
  std::shared_ptr<Oscillator> shiftOscillator;
  std::shared_ptr<Oscillator> rotationOscillator;

  KaleidoscopeSettings(std::string shaderId, json j) :
  sides(std::make_shared<Parameter>("sides", shaderId, 1.0,  1.0, 6.0)),
  shift(std::make_shared<Parameter>("shift", shaderId, 0.0,  0.0, 1.0)),
  rotation(std::make_shared<Parameter>("rotation", shaderId, 0.001,  1.0, TWO_PI)),
  sidesOscillator(std::make_shared<Oscillator>(sides)),
  shiftOscillator(std::make_shared<Oscillator>(shift)),
  rotationOscillator(std::make_shared<Oscillator>(rotation)),
  ShaderSettings(shaderId) {
    parameters = {sides, shift, rotation};
    oscillators = {sidesOscillator, shiftOscillator, rotationOscillator};
    load(j);
  };
};

struct KaleidoscopeShader : public Shader {
  ofShader shader;
  KaleidoscopeSettings *settings;
  KaleidoscopeShader(KaleidoscopeSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shadersGL2/new/kaleidoscope");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("sides", settings->sides->value);
    shader.setUniform1f("shift", settings->shift->value);
    shader.setUniform1f("rotation", settings->rotation->value);
    shader.setUniformTexture("tex", *frame, 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  ShaderType type() override {
    return ShaderType::ShaderTypeKaleidoscope;
  }

  void drawSettings() override {
    CommonViews::H3Title("Kaleidoscope");

    CommonViews::Slider("Sides", "##sides", settings->sides);
    CommonViews::Slider("Shift", "##shift", settings->shift);
    CommonViews::Slider("Rotation", "##rotation", settings->rotation);
    
    CommonViews::ModulationSelector(settings->sides);
    CommonViews::MidiSelector(settings->sides);
  }
};


#endif /* KaleidoscopeShader_h */
