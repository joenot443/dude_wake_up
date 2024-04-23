//
//  SixteenBitShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SixteenBitShader_hpp
#define SixteenBitShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SixteenBitSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderWaveformOscillator;

  SixteenBitSettings(std::string shaderId, json j, std::string name) : shaderValue(std::make_shared<Parameter>("shaderValue", 1.0, -1.0, 2.0)),
                                                     shaderWaveformOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
                                                     ShaderSettings(shaderId, j, name)
  {
    parameters = {shaderValue};
    oscillators = {shaderWaveformOscillator};
    load(j);
    registerParameters();
  };
};

class SixteenBitShader : public Shader
{
public: 
  SixteenBitSettings *settings;
  SixteenBitShader(SixteenBitSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override
  {
    shader.load("shaders/16bit");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
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
    return ShaderType16bit;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("16bit");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderWaveformOscillator);
  }
};

#endif /* SixteenBitShader_hpp */
