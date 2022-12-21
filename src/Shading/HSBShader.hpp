//
//  HSBShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HSBShader_hpp
#define HSBShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "Shader.hpp"
#include <stdio.h>

// Basic
struct HSBSettings: public ShaderSettings {
  std::string shaderId;
  std::shared_ptr<Parameter> hue;
  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<Parameter> brightness;
  
  std::shared_ptr<Oscillator> hueOscillator;
  std::shared_ptr<Oscillator> saturationOscillator;
  std::shared_ptr<Oscillator> brightnessOscillator;
  
  HSBSettings(std::string shaderId, json j) :
  shaderId(shaderId),
  hue(std::make_shared<Parameter>("Hue", shaderId, 0.0, -1.0, 2.0)),
  saturation(std::make_shared<Parameter>("Saturation", shaderId, 1.0, -1.0, 3.0)),
  brightness(std::make_shared<Parameter>("Brightness", shaderId, 1.0, -1.0, 3.0)),
  hueOscillator(std::make_shared<Oscillator>(hue)),
  saturationOscillator(std::make_shared<Oscillator>(saturation)),
  brightnessOscillator(std::make_shared<Oscillator>(brightness)),
  ShaderSettings(shaderId)
  {
    parameters = {hue, saturation, brightness};
    oscillators = {hueOscillator, saturationOscillator, brightnessOscillator};
    load(j);
  }
};

struct HSBShader: Shader {
  
  ofShader shader;
public:
  HSBShader(HSBSettings *settings) : Shader(settings), settings(settings) {};
  
  HSBSettings *settings;
  
  void setup() override {
    shader.load("shaders/hsb");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("hsbScalar", settings->hue->value, settings->saturation->value, settings->brightness->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderTypeHSB;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Basic (HSB)");
    
    CommonViews::ShaderParameter(settings->hue, settings->hueOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* HSBShader_hpp */
