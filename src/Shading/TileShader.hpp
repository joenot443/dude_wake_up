
//
//  TileShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/13/22.
//

#ifndef TileShader_h
#define TileShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TileSettings : public ShaderSettings {
  std::shared_ptr<Parameter> repeat;
  std::shared_ptr<Parameter> mirror;

  std::shared_ptr<Oscillator> repeatOscillator;

  TileSettings(std::string shaderId, json j) :
  repeat(std::make_shared<Parameter>("repeat", shaderId, 4.0, 1.0, 10.0)),
  mirror(std::make_shared<Parameter>("mirror", shaderId, 0.0, 0.0, 1.0)),
  repeatOscillator(std::make_shared<WaveformOscillator>(repeat)),
  ShaderSettings(shaderId) {
    parameters = {repeat, mirror};
    oscillators = {repeatOscillator};
    load(j);
  };
};

struct TileShader : public Shader {
  ofShader shader;
  TileSettings *settings;
  TileShader(TileSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Tile");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("repeat", settings->repeat->value);
    shader.setUniform1i("mirror", settings->mirror->boolValue);
    shader.setUniformTexture("tex", *frame, 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  ShaderType type() override {
    return ShaderType::ShaderTypeTile;
  }

  void drawSettings() override {
    CommonViews::ShaderCheckbox(settings->mirror);
    CommonViews::ShaderParameter(settings->repeat, settings->repeatOscillator);
  }
};

#endif /* TileShader_h */
