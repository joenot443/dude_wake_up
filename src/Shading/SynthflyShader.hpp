//
//  SynthflyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SynthflyShader_hpp
#define SynthflyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SynthflySettings: public ShaderSettings {
  // Core visual parameters
  std::shared_ptr<Parameter> planeDistance;  // Distance between planes
  std::shared_ptr<Parameter> maxPlanes;      // Number of rendered planes
  std::shared_ptr<Parameter> fadeStart;      // Where planes start fading
  std::shared_ptr<Parameter> alphaCutoff;    // Transparency threshold

  // Noise and pattern parameters
  std::shared_ptr<Parameter> octaveScale;    // Scale factor between noise octaves
  std::shared_ptr<Parameter> freqScale;      // Frequency scaling for noise
  std::shared_ptr<Parameter> channelHeight;  // Height of sun channels
  std::shared_ptr<Parameter> sunScale;       // Overall scale of sun effect

  // Core oscillators
  std::shared_ptr<WaveformOscillator> planeDistanceOscillator;
  std::shared_ptr<WaveformOscillator> maxPlanesOscillator;
  std::shared_ptr<WaveformOscillator> fadeStartOscillator;
  std::shared_ptr<WaveformOscillator> alphaCutoffOscillator;

  // Effect oscillators
  std::shared_ptr<WaveformOscillator> octaveScaleOscillator;
  std::shared_ptr<WaveformOscillator> freqScaleOscillator;
  std::shared_ptr<WaveformOscillator> channelHeightOscillator;
  std::shared_ptr<WaveformOscillator> sunScaleOscillator;

  SynthflySettings(std::string shaderId, json j) :
  // Initialize core parameters
  planeDistance(std::make_shared<Parameter>("Plane Distance", 0.5, 0.1, 2.0)),
  maxPlanes(std::make_shared<Parameter>("Max Planes", 24.0, 4.0, 48.0)),
  fadeStart(std::make_shared<Parameter>("Fade Start", 22.0, 2.0, 47.0)),
  alphaCutoff(std::make_shared<Parameter>("Alpha Cutoff", 0.95, 0.5, 1.0)),
  
  // Initialize effect parameters
  octaveScale(std::make_shared<Parameter>("Octave Scale", 0.35, 0.1, 1.0)),
  freqScale(std::make_shared<Parameter>("Frequency Scale", 1.8, 0.5, 4.0)),
  channelHeight(std::make_shared<Parameter>("Channel Height", 0.0125, 0.001, 0.05)),
  sunScale(std::make_shared<Parameter>("Sun Scale", 25.0, 5.0, 50.0)),
  
  // Initialize core oscillators
  planeDistanceOscillator(std::make_shared<WaveformOscillator>(planeDistance)),
  maxPlanesOscillator(std::make_shared<WaveformOscillator>(maxPlanes)),
  fadeStartOscillator(std::make_shared<WaveformOscillator>(fadeStart)),
  alphaCutoffOscillator(std::make_shared<WaveformOscillator>(alphaCutoff)),
  
  // Initialize effect oscillators
  octaveScaleOscillator(std::make_shared<WaveformOscillator>(octaveScale)),
  freqScaleOscillator(std::make_shared<WaveformOscillator>(freqScale)),
  channelHeightOscillator(std::make_shared<WaveformOscillator>(channelHeight)),
  sunScaleOscillator(std::make_shared<WaveformOscillator>(sunScale)),
  
  ShaderSettings(shaderId, j, "Synthfly") {
    parameters = { 
      planeDistance, maxPlanes, fadeStart, alphaCutoff,
      octaveScale, freqScale, channelHeight, sunScale 
    };
    oscillators = { 
      planeDistanceOscillator, maxPlanesOscillator, fadeStartOscillator, alphaCutoffOscillator,
      octaveScaleOscillator, freqScaleOscillator, channelHeightOscillator, sunScaleOscillator 
    };
    load(j);
    registerParameters();
  };
};

struct SynthflyShader: Shader {
  SynthflySettings *settings;
  SynthflyShader(SynthflySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Synthfly");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    // Pass core parameters to the shader
    shader.setUniform1f("planeDistance", settings->planeDistance->value);
    shader.setUniform1f("maxPlanes", settings->maxPlanes->value);
    shader.setUniform1f("fadeStart", settings->fadeStart->value);
    shader.setUniform1f("alphaCutoff", settings->alphaCutoff->value);
    
    // Pass effect parameters to the shader
    shader.setUniform1f("octaveScale", settings->octaveScale->value);
    shader.setUniform1f("freqScale", settings->freqScale->value);
    shader.setUniform1f("channelHeight", settings->channelHeight->value);
    shader.setUniform1f("sunScale", settings->sunScale->value);
    
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
    return ShaderTypeSynthfly;
  }

  void drawSettings() override {
    CommonViews::H3Title("Synthfly");
    
    // Core visual parameters
    CommonViews::H4Title("Core Parameters");
    CommonViews::ShaderParameter(settings->planeDistance, settings->planeDistanceOscillator);
    CommonViews::ShaderParameter(settings->maxPlanes, settings->maxPlanesOscillator);
    CommonViews::ShaderParameter(settings->fadeStart, settings->fadeStartOscillator);
    CommonViews::ShaderParameter(settings->alphaCutoff, settings->alphaCutoffOscillator);
    
    // Effect parameters
    CommonViews::H4Title("Effect Parameters");
    CommonViews::ShaderParameter(settings->octaveScale, settings->octaveScaleOscillator);
    CommonViews::ShaderParameter(settings->freqScale, settings->freqScaleOscillator);
    CommonViews::ShaderParameter(settings->channelHeight, settings->channelHeightOscillator);
    CommonViews::ShaderParameter(settings->sunScale, settings->sunScaleOscillator);
  }
};

#endif /* SynthflyShader_hpp */
