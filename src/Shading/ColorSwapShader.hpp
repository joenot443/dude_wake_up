//
//  ColorSwapShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorSwapShader_hpp
#define ColorSwapShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ColorSwapSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mainRGB;
  std::shared_ptr<Parameter> auxRGB;
  std::shared_ptr<Parameter> blend;
  
  std::shared_ptr<WaveformOscillator> blendOscillator;
  
  ColorSwapSettings(std::string shaderId, json j, std::string name) :
  mainRGB(std::make_shared<Parameter>("Main RGB", 0.0, 0.0, 2.0, ParameterType_Int)),
  auxRGB(std::make_shared<Parameter>("Aux RGB", 0.0, 0.0, 2.0, ParameterType_Int)),
  blend(std::make_shared<Parameter>("Blend", 0.5, 0.0, 1.0)),
  blendOscillator(std::make_shared<WaveformOscillator>(blend)),
  ShaderSettings(shaderId, j, name) {
    parameters = { mainRGB, auxRGB, blend };
    oscillators = { blendOscillator };
    load(j);
    registerParameters();
  };
};

struct ColorSwapShader: Shader {
  ColorSwapSettings *settings;
  ColorSwapShader(ColorSwapSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/ColorSwap");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (hasInputAtSlot(InputSlotTwo)) {
      shader.setUniformTexture("tex2", inputAtSlot(InputSlotTwo)->frame()->getTexture(), 8);
    }
    shader.setUniform1i("mainRGB", settings->mainRGB->intValue);
    shader.setUniform1i("auxRGB", settings->auxRGB->intValue);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("blend", settings->blend->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return 2;
  }
  
  ShaderType type() override {
    return ShaderTypeColorSwap;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderIntParameter(settings->mainRGB);
    CommonViews::ShaderIntParameter(settings->auxRGB);
    CommonViews::ShaderParameter(settings->blend, settings->blendOscillator);
  }
};

#endif
