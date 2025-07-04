//
//  BlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlurShader_hpp
#define BlurShader_hpp

#include "WaveformOscillator.hpp"
#include "ofMain.h"

#include "Math.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include <stdio.h>

struct BlurSettings : public ShaderSettings
{
public:
  std::string shaderId;
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<Oscillator> radiusOscillator;
  
  BlurSettings(std::string shaderId, json j, std::string name)
  : radius(std::make_shared<Parameter>("Amount", 0.01, 0.0,
                                     10.0)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  shaderId(shaderId), ShaderSettings(shaderId, j, name)
  {
    parameters = {radius};
    oscillators = {radiusOscillator};
    audioReactiveParameter = radius;
    load(j);
    registerParameters();
  }
};

class BlurShader : public Shader
{
public:
  BlurSettings *settings;
  
  
public:
  BlurShader(BlurSettings *settings) : Shader(settings), settings(settings) {}
  void setup() override
  {
    shader.load("shaders/blur");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("radius", settings->radius->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  int inputCount() override {
    return 1;
  }
  ShaderType type() override { return ShaderTypeBlur; }
  
  void drawSettings() override
  {
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
  }
};

#endif
