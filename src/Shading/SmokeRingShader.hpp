//
//  SmokeRingShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SmokeRingShader_hpp
#define SmokeRingShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SmokeRingSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<ValueOscillator> radiusOscillator;

  std::shared_ptr<Parameter> colorFactor;
  std::shared_ptr<ValueOscillator> colorFactorOscillator;

  SmokeRingSettings(std::string shaderId, json j) : radius(std::make_shared<Parameter>("radius", 1.0, 0.0, 2.0)),
                                                    colorFactor(std::make_shared<Parameter>("colorFactor", 0.0, -2.0, 2.0)),
                                                    colorFactorOscillator(std::make_shared<ValueOscillator>(colorFactor)),
                                                    radiusOscillator(std::make_shared<ValueOscillator>(radius)),
                                                    ShaderSettings(shaderId, j)
  {
    parameters = {radius, colorFactor};
    oscillators = {radiusOscillator, colorFactorOscillator};
    load(j);
    registerParameters();
  };
};

struct SmokeRingShader : Shader
{
  SmokeRingSettings *settings;
  SmokeRingShader(SmokeRingSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override
  {
    shader.load("shaders/SmokeRing");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("radius", settings->radius->value);
    shader.setUniform1f("colorFactor", settings->colorFactor->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

  ShaderType type() override
  {
    return ShaderTypeSmokeRing;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("SmokeRing");

    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
    CommonViews::ShaderParameter(settings->colorFactor, settings->colorFactorOscillator);
  }
};

#endif /* SmokeRingShader_hpp */
