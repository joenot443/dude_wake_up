//
//  TwistedTripShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TwistedTripShader_hpp
#define TwistedTripShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TwistedTripSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  
  // New parameters for geometric control
  std::shared_ptr<Parameter> patternDensity;  // Controls the density of the geometric pattern
  std::shared_ptr<WaveformOscillator> patternDensityOscillator;
  
  std::shared_ptr<Parameter> twistAmount;     // Controls the intensity of the twisting effect
  std::shared_ptr<WaveformOscillator> twistAmountOscillator;
  
  // New parameters for color control
  std::shared_ptr<Parameter> colorShift;      // Shifts the color frequencies
  std::shared_ptr<WaveformOscillator> colorShiftOscillator;
  
  std::shared_ptr<Parameter> patternScale;    // Controls the overall scale of the pattern
  std::shared_ptr<WaveformOscillator> patternScaleOscillator;

  TwistedTripSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 10.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  patternDensity(std::make_shared<Parameter>("Pattern Density", 1.0, 0.1, 2.0)),
  patternDensityOscillator(std::make_shared<WaveformOscillator>(patternDensity)),
  twistAmount(std::make_shared<Parameter>("Twist Amount", 0.75, 0.0, 2.0)),
  twistAmountOscillator(std::make_shared<WaveformOscillator>(twistAmount)),
  colorShift(std::make_shared<Parameter>("Color Shift", 0.0, -1.0, 1.0)),
  colorShiftOscillator(std::make_shared<WaveformOscillator>(colorShift)),
  patternScale(std::make_shared<Parameter>("Pattern Scale", 1.0, 0.5, 2.0)),
  patternScaleOscillator(std::make_shared<WaveformOscillator>(patternScale)),
  ShaderSettings(shaderId, j, "TwistedTrip") {
    parameters = { speed, patternDensity, twistAmount, colorShift, patternScale };
    oscillators = { speedOscillator, patternDensityOscillator, twistAmountOscillator, 
                   colorShiftOscillator, patternScaleOscillator };
    load(j);
    registerParameters();
  };
};

struct TwistedTripShader: Shader {
  TwistedTripSettings *settings;
  TwistedTripShader(TwistedTripSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/TwistedTrip");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("patternDensity", settings->patternDensity->value);
    shader.setUniform1f("twistAmount", settings->twistAmount->value);
    shader.setUniform1f("colorShift", settings->colorShift->value);
    shader.setUniform1f("patternScale", settings->patternScale->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
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
    return ShaderTypeTwistedTrip;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->patternDensity, settings->patternDensityOscillator);
    CommonViews::ShaderParameter(settings->twistAmount, settings->twistAmountOscillator);
    CommonViews::ShaderParameter(settings->colorShift, settings->colorShiftOscillator);
    CommonViews::ShaderParameter(settings->patternScale, settings->patternScaleOscillator);
  }
};

#endif
