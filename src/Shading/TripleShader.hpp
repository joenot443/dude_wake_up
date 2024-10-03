//
//  TripleShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TripleShader_hpp
#define TripleShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "BlendShader.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TripleSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<WaveformOscillator> scaleOscillator;

  std::shared_ptr<Parameter> xShift;
  std::shared_ptr<WaveformOscillator> xShiftOscillator;

  std::shared_ptr<Parameter> yShift;
  std::shared_ptr<WaveformOscillator> yShiftOscillator;

  std::shared_ptr<Parameter> drawOriginal;
  std::shared_ptr<Parameter> drawLeft;
  std::shared_ptr<Parameter> drawRight;
  std::shared_ptr<Parameter> drawCenter;
  
  std::shared_ptr<Parameter> blendMode;

  TripleSettings(std::string shaderId, json j, std::string name) : scale(std::make_shared<Parameter>("scale", 1.0, 0.0, 5.0)),
                                                 scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
                                                 xShift(std::make_shared<Parameter>("xShift", 0.0, -2.0, 2.0)),
                                                 xShiftOscillator(std::make_shared<WaveformOscillator>(xShift)),
                                                 yShift(std::make_shared<Parameter>("yShift", 0.0, -2.0, 2.0)),
                                                 yShiftOscillator(std::make_shared<WaveformOscillator>(yShift)),
                                                 drawOriginal(std::make_shared<Parameter>("drawOriginal", ParameterType_Bool)),
                                                 drawLeft(std::make_shared<Parameter>("drawLeft", ParameterType_Bool)),
                                                 drawRight(std::make_shared<Parameter>("drawRight", ParameterType_Bool)),
                                                 drawCenter(std::make_shared<Parameter>("drawCenter", ParameterType_Bool)),
  blendMode(std::make_shared<Parameter>("Blend Mode", 0.0, 0.0, 13.0)),                                                 ShaderSettings(shaderId, j, name)
  {
    parameters = {scale, xShift, yShift, drawOriginal, drawLeft, drawRight, drawCenter, blendMode };
    oscillators = {scaleOscillator, xShiftOscillator, yShiftOscillator};
    load(j);
    registerParameters();
  };
};

struct TripleShader : Shader
{
  TripleSettings *settings;
  TripleShader(TripleSettings *settings) : settings(settings), Shader(settings){};

  void setup() override
  {
    shader.load("shaders/Triple");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("scale", settings->scale->value);
    shader.setUniform1f("xShift", settings->xShift->value);
    shader.setUniform1f("yShift", settings->yShift->value);
    shader.setUniform1i("drawLeft", settings->drawLeft->boolValue);
    shader.setUniform1i("blendMode", settings->blendMode->intValue);
    shader.setUniform1i("drawRight", settings->drawRight->boolValue);
    shader.setUniform1i("drawCenter", settings->drawCenter->boolValue);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    if (settings->drawOriginal->boolValue)
    {
      frame->draw(0, 0);
    }
    
    canvas->end();
  }

  void clear() override
  {
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { 
    return ShaderTypeTriple;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("Triple");

    CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
    CommonViews::ShaderParameter(settings->xShift, settings->xShiftOscillator);
    CommonViews::ShaderParameter(settings->yShift, settings->yShiftOscillator);
    CommonViews::ShaderCheckbox(settings->drawOriginal);
    CommonViews::ShaderCheckbox(settings->drawLeft);
    CommonViews::ShaderCheckbox(settings->drawCenter);
    CommonViews::ShaderCheckbox(settings->drawRight);
    CommonViews::BlendModeSelector(settings->blendMode);
  }
};

#endif /* TripleShader_hpp */
