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
  std::shared_ptr<Parameter> pixelFactor;
  std::shared_ptr<WaveformOscillator> pixelFactorOscillator;

  SixteenBitSettings(std::string shaderId, json j, std::string name) : pixelFactor(std::make_shared<Parameter>("pixelFactor", 150.0, 100.0, 400.0)),
                                                     pixelFactorOscillator(std::make_shared<WaveformOscillator>(pixelFactor)),
                                                     ShaderSettings(shaderId, j, name)
  {
    parameters = {pixelFactor};
    oscillators = {pixelFactorOscillator};
    audioReactiveParameter = pixelFactor;
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
    shader.setUniform1f("pixelFactor", settings->pixelFactor->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { 
    return ShaderType16bit;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("16bit");

    CommonViews::ShaderParameter(settings->pixelFactor, settings->pixelFactorOscillator);
  }
};

#endif /* SixteenBitShader_hpp */
