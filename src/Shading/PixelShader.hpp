//
//  PixelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelShader_hpp
#define PixelShader_hpp

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Pixelate

struct PixelSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> size;
  std::shared_ptr<Parameter> gradient;

  std::shared_ptr<Oscillator> sizeOscillator;
  std::shared_ptr<Oscillator> gradientOscillator;

  PixelSettings(std::string shaderId, json j, std::string name)
  : size(std::make_shared<Parameter>("Size", 24.1, 1.0, 300.0)),
  gradient(std::make_shared<Parameter>("Gradient", 0.0, 0.0, 1.0)),
  sizeOscillator(std::make_shared<WaveformOscillator>(size)),
  gradientOscillator(std::make_shared<WaveformOscillator>(gradient)),
  ShaderSettings(shaderId, j, name) {
    parameters = { size, gradient };
    oscillators = { sizeOscillator, gradientOscillator };
    audioReactiveParameter = size;
    load(j);
    registerParameters();
  }
};

class PixelShader : public Shader {
public:
  
  PixelSettings *settings;
  PixelShader(PixelSettings *settings) : settings(settings), Shader(settings){};
  

  
  void setup() override {
    shader.load("shaders/Pixel");
    
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("size", settings->size->value);
    shader.setUniform1f("gradient", settings->gradient->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  bool enabled() override { return true; }
  
  int inputCount() override {
    return 1;
  }
  ShaderType type() override { return ShaderTypePixelate; }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
    CommonViews::ShaderParameter(settings->gradient, settings->gradientOscillator);
  }
};

#endif
