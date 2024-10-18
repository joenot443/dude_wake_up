
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
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TileSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> repeat;
  std::shared_ptr<Parameter> spacing;
  std::shared_ptr<Parameter> mirror;
  
  std::shared_ptr<Oscillator> repeatOscillator;
  std::shared_ptr<Oscillator> spacingOscillator;
  
  TileSettings(std::string shaderId, json j, std::string name) :
  repeat(std::make_shared<Parameter>("repeat", 4.0, 1.0, 10.0)),
  spacing(std::make_shared<Parameter>("spacing", 5.0, 0.0, 50.0)),
  mirror(std::make_shared<Parameter>("mirror", ParameterType_Bool)),
  repeatOscillator(std::make_shared<WaveformOscillator>(repeat)),
  ShaderSettings(shaderId, j, name) {
    parameters = {repeat, mirror, spacing};
    oscillators = {repeatOscillator, spacingOscillator};
    load(j);
    registerParameters();
  };
};

class TileShader : public Shader {
public:

  TileSettings *settings;
  TileShader(TileSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
#ifdef DEBUG
    shader.load("shaders/Tile");
#endif
#ifdef RELEASE
    shader.load("shaders/Tile");
#endif
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("repeat", settings->repeat->value);
    shader.setUniform1f("spacing", settings->spacing->value);
    shader.setUniform1i("mirror", settings->mirror->boolValue);
    shader.setUniformTexture("tex", *frame, 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderType::ShaderTypeTile;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderCheckbox(settings->mirror);
    CommonViews::ShaderParameter(settings->repeat, settings->repeatOscillator);
    CommonViews::ShaderParameter(settings->spacing, settings->spacingOscillator);
  }
};

#endif /* TileShader_h */
