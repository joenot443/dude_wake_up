//
//  ColorPassShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorPassShader_hpp
#define ColorPassShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ColorPassSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> lowHue;
  std::shared_ptr<Parameter> highHue;

  std::shared_ptr<Parameter> drawMiss;
  
  std::shared_ptr<Oscillator> lowHueOsc;
  std::shared_ptr<Oscillator> highHueOsc;
  
  ColorPassSettings(std::string shaderId, json j, std::string name)
  : lowHue(std::make_shared<Parameter>("Low Hue", 0.0, 0.0, 1.0)),
  highHue(
          std::make_shared<Parameter>("High Hue", 0.5, 0.0, 1.0)),
  drawMiss(std::make_shared<Parameter>("Draw Miss", 0.0, 0.0, 1.0, ParameterType_Bool)),
  lowHueOsc(std::make_shared<WaveformOscillator>(lowHue)),
  highHueOsc(std::make_shared<WaveformOscillator>(highHue)),
  ShaderSettings(shaderId, j, name){
    parameters = { lowHue, highHue, drawMiss };
    oscillators = { lowHueOsc, highHueOsc };
    load(j);
    registerParameters();
  };
};

class ColorPassShader : public Shader {
public:
  
  ColorPassSettings *settings;
  ColorPassShader(ColorPassSettings *settings)
  : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/ColorPass");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("hue1", settings->lowHue->value);
    shader.setUniform1f("hue2", settings->highHue->value);
    shader.setUniform1i("invert", 0);
    shader.setUniform1i("drawMiss", settings->drawMiss->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override { return ShaderTypeColorPass; }
  
  void drawSettings() override {
    CommonViews::RangeSlider("Hue", settings->lowHue->paramId, settings->lowHue, settings->highHue, "Threshold", false);
    CommonViews::ShaderCheckbox(settings->drawMiss);
  }
};

#endif
