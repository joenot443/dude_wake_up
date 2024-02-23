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
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TripleSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<ValueOscillator> scaleOscillator;

  std::shared_ptr<Parameter> xShift;
  std::shared_ptr<ValueOscillator> xShiftOscillator;

  std::shared_ptr<Parameter> yShift;
  std::shared_ptr<ValueOscillator> yShiftOscillator;

  std::shared_ptr<Parameter> drawOriginal;
  std::shared_ptr<Parameter> drawLeft;
  std::shared_ptr<Parameter> drawRight;
  std::shared_ptr<Parameter> drawCenter;

  TripleSettings(std::string shaderId, json j) : scale(std::make_shared<Parameter>("scale", 1.0, 0.0, 5.0)),
                                                 scaleOscillator(std::make_shared<ValueOscillator>(scale)),
                                                 xShift(std::make_shared<Parameter>("xShift", 0.0, -2.0, 2.0)),
                                                 xShiftOscillator(std::make_shared<ValueOscillator>(xShift)),
                                                 yShift(std::make_shared<Parameter>("yShift", 0.0, -2.0, 2.0)),
                                                 yShiftOscillator(std::make_shared<ValueOscillator>(yShift)),
                                                 drawOriginal(std::make_shared<Parameter>("drawOriginal", true)),
                                                 drawLeft(std::make_shared<Parameter>("drawLeft", true)),
                                                 drawRight(std::make_shared<Parameter>("drawRight", true)),
                                                 drawCenter(std::make_shared<Parameter>("drawCenter", true)),
                                                 ShaderSettings(shaderId, j)
  {
    parameters = {scale, xShift, yShift, drawOriginal, drawLeft, drawRight, drawCenter};
    oscillators = {scaleOscillator, xShiftOscillator, yShiftOscillator};
    load(j);
    registerParameters();
  };
};

struct TripleShader : Shader
{
  TripleSettings *settings;
  TripleShader(TripleSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
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

  ShaderType type() override
  {
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
  }
};

#endif /* TripleShader_hpp */
