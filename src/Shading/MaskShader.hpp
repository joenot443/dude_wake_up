//
//  MaskShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MaskShader_hpp
#define MaskShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct MaskSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> blend;
  std::shared_ptr<Parameter> swap; // Add swap parameter
  std::shared_ptr<Oscillator> blendOscillator;
  
  MaskSettings(std::string shaderId, json j, std::string name) 
    : blend(std::make_shared<Parameter>("Blend", 1.0, 0.0, 1.0)),
      swap(std::make_shared<Parameter>("Swap", false, ParameterType_Bool)), // Initialize swap parameter
      blendOscillator(std::make_shared<WaveformOscillator>(blend)),
      ShaderSettings(shaderId, j, name)
  {
    parameters = {blend, swap}; // Add swap to parameters
    oscillators = {blendOscillator};
    load(j);
    registerParameters();
  };
};

struct MaskShader : Shader
{
  MaskSettings *settings;
  MaskShader(MaskSettings *settings) : settings(settings), Shader(settings){};

  bool swap;

  void setup() override
  {
    shader.load("shaders/Mask");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);

    if (settings->swap->boolValue) {
      shader.setUniformTexture("maskTex", frame->getTexture(), 4);
      if (hasInputAtSlot(InputSlotTwo))
      {
        shader.setUniformTexture("tex", inputAtSlot(InputSlotTwo)->frame()->getTexture(), 12);
      }
    } else {
      shader.setUniformTexture("tex", frame->getTexture(), 4);
      if (hasInputAtSlot(InputSlotTwo))
      {
        shader.setUniformTexture("maskTex", inputAtSlot(InputSlotTwo)->frame()->getTexture(), 12);
      }
    }

    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("blend", settings->blend->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

  int inputCount() override {
    return 2;
  }
  
  ShaderType type() override {
    return ShaderTypeMask;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("Mask");
    CommonViews::ShaderParameter(settings->blend, settings->blendOscillator);
    CommonViews::ShaderCheckbox(settings->swap); // Add swap to settings view
  }
};

#endif
