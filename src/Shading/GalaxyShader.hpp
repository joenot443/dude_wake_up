//
//  GalaxyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GalaxyShader_hpp
#define GalaxyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "WaveformOscillator.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GalaxySettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Parameter> colorScale;
  std::shared_ptr<Parameter> starCount;
  std::shared_ptr<Parameter> moonPhase;
  std::shared_ptr<Parameter> skyBrightness;
  std::shared_ptr<Parameter> galaxyScale;
  std::shared_ptr<Parameter> cameraSpeed;

  std::shared_ptr<WaveformOscillator> colorScaleOscillator;
  std::shared_ptr<WaveformOscillator> starCountOscillator;
  std::shared_ptr<WaveformOscillator> moonPhaseOscillator;
  std::shared_ptr<WaveformOscillator> skyBrightnessOscillator;
  std::shared_ptr<WaveformOscillator> galaxyScaleOscillator;
  std::shared_ptr<WaveformOscillator> cameraSpeedOscillator;
  
  GalaxySettings(std::string shaderId, json j, std::string name) :
    colorScale(std::make_shared<Parameter>("Color Scale", 1.0, 0.0, 2.0)),
    starCount(std::make_shared<Parameter>("Star Count", 1.0, 0.0, 5.0)),
    moonPhase(std::make_shared<Parameter>("Moon Phase", 0.0, 0.0, 1.0)),
    skyBrightness(std::make_shared<Parameter>("Sky Brightness", 1.0, 0.0, 5.0)),
    galaxyScale(std::make_shared<Parameter>("Galaxy Scale", 1.0, 0.1, 5.0)),
    cameraSpeed(std::make_shared<Parameter>("Camera Speed", 1.0, 0.0, 5.0)),
    colorScaleOscillator(std::make_shared<WaveformOscillator>(colorScale)),
    starCountOscillator(std::make_shared<WaveformOscillator>(starCount)),
    moonPhaseOscillator(std::make_shared<WaveformOscillator>(moonPhase)),
    skyBrightnessOscillator(std::make_shared<WaveformOscillator>(skyBrightness)),
    galaxyScaleOscillator(std::make_shared<WaveformOscillator>(galaxyScale)),
    cameraSpeedOscillator(std::make_shared<WaveformOscillator>(cameraSpeed)),
    ShaderSettings(shaderId, j, name) {
    parameters = {colorScale, starCount, moonPhase, skyBrightness, galaxyScale, cameraSpeed};
    oscillators = {colorScaleOscillator, starCountOscillator, moonPhaseOscillator, skyBrightnessOscillator, galaxyScaleOscillator, cameraSpeedOscillator};
    load(j);
    registerParameters();
    
  };
};

class GalaxyShader: public Shader {
public:
  
  GalaxySettings *settings;
  GalaxyShader(GalaxySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Galaxy");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("colorScale", settings->colorScale->value);
    shader.setUniform1f("starCount", settings->starCount->value);
    shader.setUniform1f("moonPhase", settings->moonPhase->value);
    shader.setUniform1f("skyBrightness", settings->skyBrightness->value);
    shader.setUniform1f("galaxyScale", settings->galaxyScale->value);
    shader.setUniform1f("cameraSpeed", settings->cameraSpeed->value);
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
    return ShaderTypeGalaxy;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->colorScale, settings->colorScaleOscillator);
    CommonViews::ShaderParameter(settings->starCount, settings->starCountOscillator);
    CommonViews::ShaderParameter(settings->moonPhase, settings->moonPhaseOscillator);
    CommonViews::ShaderParameter(settings->skyBrightness, settings->skyBrightnessOscillator);
    CommonViews::ShaderParameter(settings->galaxyScale, settings->galaxyScaleOscillator);
    CommonViews::ShaderParameter(settings->cameraSpeed, settings->cameraSpeedOscillator);
  }
};

#endif
