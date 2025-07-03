//
//  CosmosShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CosmosShader_hpp
#define CosmosShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CosmosSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  // Cosmos specific params
  std::shared_ptr<Parameter> warp;
  std::shared_ptr<WaveformOscillator> warpOscillator;
  std::shared_ptr<Parameter> sunSize;
  std::shared_ptr<WaveformOscillator> sunSizeOscillator;
  std::shared_ptr<Parameter> sunColorR;
  std::shared_ptr<WaveformOscillator> sunColorROscillator;
  std::shared_ptr<Parameter> sunColorG;
  std::shared_ptr<WaveformOscillator> sunColorGOscillator;
  std::shared_ptr<Parameter> sunColorB;
  std::shared_ptr<WaveformOscillator> sunColorBOscillator;
  std::shared_ptr<Parameter> sunSpread;
  std::shared_ptr<WaveformOscillator> sunSpreadOscillator;
  std::shared_ptr<Parameter> starCount;
  std::shared_ptr<WaveformOscillator> starCountOscillator;
  std::shared_ptr<Parameter> starScale;
  std::shared_ptr<WaveformOscillator> starScaleOscillator;
  std::shared_ptr<Parameter> starBrightness;
  std::shared_ptr<WaveformOscillator> starBrightnessOscillator;


  CosmosSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  warp(std::make_shared<Parameter>("Warp", 0.03, 0.0, 0.2)),
  warpOscillator(std::make_shared<WaveformOscillator>(warp)),
  sunSize(std::make_shared<Parameter>("Sun Size", 3.25, 0.0, 10.0)),
  sunSizeOscillator(std::make_shared<WaveformOscillator>(sunSize)),
  sunColorR(std::make_shared<Parameter>("Sun Color R", 1.0, 0.0, 5.0)),
  sunColorROscillator(std::make_shared<WaveformOscillator>(sunColorR)),
  sunColorG(std::make_shared<Parameter>("Sun Color G", 0.5, 0.0, 5.0)),
  sunColorGOscillator(std::make_shared<WaveformOscillator>(sunColorG)),
  sunColorB(std::make_shared<Parameter>("Sun Color B", 2.1, 0.0, 5.0)),
  sunColorBOscillator(std::make_shared<WaveformOscillator>(sunColorB)),
  sunSpread(std::make_shared<Parameter>("Sun Spread", 0.4, 0.1, 3.0)),
  sunSpreadOscillator(std::make_shared<WaveformOscillator>(sunSpread)),
  starCount(std::make_shared<Parameter>("Star Count", 3.0, 0.0, 15.0)),
  starCountOscillator(std::make_shared<WaveformOscillator>(starCount)),
  starScale(std::make_shared<Parameter>("Star Scale", 1.0, 0.0, 5.0)),
  starScaleOscillator(std::make_shared<WaveformOscillator>(starScale)),
  starBrightness(std::make_shared<Parameter>("Star Brightness", 0.7, 0.0, 2.0)),
  starBrightnessOscillator(std::make_shared<WaveformOscillator>(starBrightness)),
  ShaderSettings(shaderId, j, "Cosmos") {
    parameters = { shaderValue, warp, sunSize, sunColorR, sunColorG, sunColorB, sunSpread, starCount, starScale, starBrightness };
    oscillators = { shaderValueOscillator, warpOscillator, sunSizeOscillator, sunColorROscillator, sunColorGOscillator, sunColorBOscillator, sunSpreadOscillator, starCountOscillator, starScaleOscillator, starBrightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct CosmosShader: Shader {
  CosmosSettings *settings;
  CosmosShader(CosmosSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Cosmos");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("u_warp", settings->warp->value);
    shader.setUniform1f("u_sunSize", settings->sunSize->value);
    shader.setUniform3f("u_sunColor", settings->sunColorR->value, settings->sunColorG->value, settings->sunColorB->value);
    shader.setUniform1f("u_sunSpread", settings->sunSpread->value);
    shader.setUniform1f("u_starCount", settings->starCount->value);
    shader.setUniform1f("u_starScale", settings->starScale->value);
    shader.setUniform1f("u_starBrightness", settings->starBrightness->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeCosmos;
  }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->warp, settings->warpOscillator);
    CommonViews::ShaderParameter(settings->sunSize, settings->sunSizeOscillator);
    CommonViews::ShaderParameter(settings->sunColorR, settings->sunColorROscillator);
    CommonViews::ShaderParameter(settings->sunColorG, settings->sunColorGOscillator);
    CommonViews::ShaderParameter(settings->sunColorB, settings->sunColorBOscillator);
    CommonViews::ShaderParameter(settings->sunSpread, settings->sunSpreadOscillator);
    CommonViews::ShaderIntParameter(settings->starCount);
    CommonViews::ShaderParameter(settings->starScale, settings->starScaleOscillator);
    CommonViews::ShaderParameter(settings->starBrightness, settings->starBrightnessOscillator);
  }
};

#endif
