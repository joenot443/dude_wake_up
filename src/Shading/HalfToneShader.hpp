//
//  HalfToneShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HalfToneShader_hpp
#define HalfToneShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct HalfToneSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<Parameter> dotSize;
  std::shared_ptr<Parameter> contrast;
  std::shared_ptr<Parameter> softness;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> radiusOscillator;
  std::shared_ptr<Oscillator> dotSizeOscillator;
  std::shared_ptr<Oscillator> contrastOscillator;
  std::shared_ptr<Oscillator> softnessOscillator;

  HalfToneSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("Speed", 0., 0., 10.)),
  radius(std::make_shared<Parameter>("Radius", 0., 0., 10.)),
  dotSize(std::make_shared<Parameter>("Dot Size", 1.48, 0.5, 3.0)),
  contrast(std::make_shared<Parameter>("Contrast", 0.888, 0.3, 1.0)),
  softness(std::make_shared<Parameter>("Softness", 0.288, 0.05, 0.5)),

  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  dotSizeOscillator(std::make_shared<WaveformOscillator>(dotSize)),
  contrastOscillator(std::make_shared<WaveformOscillator>(contrast)),
  softnessOscillator(std::make_shared<WaveformOscillator>(softness)),
  ShaderSettings(shaderId, j, name) {
    parameters = { speed, radius, dotSize, contrast, softness };
    oscillators = { speedOscillator, radiusOscillator, dotSizeOscillator, contrastOscillator, softnessOscillator };
    load(j);
    registerParameters();
  };
};

class HalfToneShader: public Shader {
public:
  
  HalfToneSettings *settings;
  HalfToneShader(HalfToneSettings *settings) : settings(settings), Shader(settings) {};

  
  void setup() override {
    shader.load("shaders/Half Tone");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("radius", settings->radius->value);
    shader.setUniform1f("dotSize", settings->dotSize->value);
    shader.setUniform1f("contrast", settings->contrast->value);
    shader.setUniform1f("softness", settings->softness->value);
    
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
    return ShaderTypeHalfTone;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
    CommonViews::ShaderParameter(settings->dotSize, settings->dotSizeOscillator);
    CommonViews::ShaderParameter(settings->contrast, settings->contrastOscillator);
    CommonViews::ShaderParameter(settings->softness, settings->softnessOscillator);
  }
};

#endif
