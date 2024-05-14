//
//  HSBShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HSBShader_hpp
#define HSBShader_hpp

#include "ofMain.h"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

// Basic
struct HSBSettings: public ShaderSettings {
	public:
  std::string shaderId;
  std::shared_ptr<Parameter> hue;
  std::shared_ptr<Parameter> saturation;
  std::shared_ptr<Parameter> brightness;
  
  std::shared_ptr<Oscillator> hueOscillator;
  std::shared_ptr<Oscillator> saturationOscillator;
  std::shared_ptr<Oscillator> brightnessOscillator;
  
  HSBSettings(std::string shaderId, json j, std::string name) :
  shaderId(shaderId),
  hue(std::make_shared<Parameter>("Hue", 0.0, 0.0, 1.0)),
  saturation(std::make_shared<Parameter>("Saturation", 1.0, 0.0, 2.0)),
  brightness(std::make_shared<Parameter>("Brightness", 1.0, 0.0, 2.0)),
  hueOscillator(std::make_shared<WaveformOscillator>(hue)),
  saturationOscillator(std::make_shared<WaveformOscillator>(saturation)),
  brightnessOscillator(std::make_shared<WaveformOscillator>(brightness)),
  ShaderSettings(shaderId, j, name)
  {
    parameters = {hue, saturation, brightness};
    oscillators = {hueOscillator, saturationOscillator, brightnessOscillator};
    load(j);
  registerParameters();
  }
};

class HSBShader: public Shader {
public:

  ofShader shader;
public:
  HSBShader(HSBSettings *settings) : Shader(settings), settings(settings) {};
  
  HSBSettings *settings;
  
  void setup() override {
    #ifdef TESTING
shader.load("shaders/hsb");
#endif
#ifdef RELEASE
shader.load("shaders/hsb");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("hsbScalar", settings->hue->value, settings->saturation->value, settings->brightness->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeHSB;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->hue, settings->hueOscillator);
    CommonViews::ShaderParameter(settings->saturation, settings->saturationOscillator);
    CommonViews::ShaderParameter(settings->brightness, settings->brightnessOscillator);
  }
};

#endif /* HSBShader_hpp */
