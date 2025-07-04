//
//  CloudShader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 12/2/22.
//

#ifndef CloudShader_h
#define CloudShader_h

#include "ofMain.h"
#include "WaveformOscillator.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CloudSettings : public ShaderSettings {
public:
  std::string shaderId;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> cloudscale;
  std::shared_ptr<Parameter> clouddark;
  std::shared_ptr<Parameter> cloudlight;
  std::shared_ptr<Parameter> cloudcover;
  std::shared_ptr<Parameter> cloudalpha;
  std::shared_ptr<Parameter> skytint;

  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<WaveformOscillator> cloudscaleOscillator;
  std::shared_ptr<WaveformOscillator> clouddarkOscillator;
  std::shared_ptr<WaveformOscillator> cloudlightOscillator;
  std::shared_ptr<WaveformOscillator> cloudcoverOscillator;
  std::shared_ptr<WaveformOscillator> cloudalphaOscillator;
  std::shared_ptr<WaveformOscillator> skytintOscillator;

  CloudSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  cloudscale(std::make_shared<Parameter>("Cloud Scale", 1.1, 0.1, 5.0)),
  clouddark(std::make_shared<Parameter>("Cloud Dark", 0.5, 0.0, 1.0)),
  cloudlight(std::make_shared<Parameter>("Cloud Light", 0.3, 0.0, 1.0)),
  cloudcover(std::make_shared<Parameter>("Cloud Cover", 0.2, 0.0, 1.0)),
  cloudalpha(std::make_shared<Parameter>("Cloud Alpha", 8.0, 0.0, 20.0)),
  skytint(std::make_shared<Parameter>("Sky Tint", 0.5, 0.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  cloudscaleOscillator(std::make_shared<WaveformOscillator>(cloudscale)),
  clouddarkOscillator(std::make_shared<WaveformOscillator>(clouddark)),
  cloudlightOscillator(std::make_shared<WaveformOscillator>(cloudlight)),
  cloudcoverOscillator(std::make_shared<WaveformOscillator>(cloudcover)),
  cloudalphaOscillator(std::make_shared<WaveformOscillator>(cloudalpha)),
  skytintOscillator(std::make_shared<WaveformOscillator>(skytint)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, cloudscale, clouddark, cloudlight, cloudcover, cloudalpha, skytint};
    oscillators = {speedOscillator, cloudscaleOscillator, clouddarkOscillator, cloudlightOscillator, cloudcoverOscillator, cloudalphaOscillator, skytintOscillator};
    load(j);
    registerParameters();
  };
};

class CloudShader: public Shader {
private:
  
  
public:
  CloudSettings *settings;
  
  CloudShader(CloudSettings *settings) :
  settings(settings),
  Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/clouds");
  }
  
  int inputCount() override {
    return 0;
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value * settings->speed->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("cloudscale", settings->cloudscale->value);
    shader.setUniform1f("clouddark", settings->clouddark->value);
    shader.setUniform1f("cloudlight", settings->cloudlight->value);
    shader.setUniform1f("cloudcover", settings->cloudcover->value);
    shader.setUniform1f("cloudalpha", settings->cloudalpha->value);
    shader.setUniform1f("skytint", settings->skytint->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->cloudscale, settings->cloudscaleOscillator);
    CommonViews::ShaderParameter(settings->clouddark, settings->clouddarkOscillator);
    CommonViews::ShaderParameter(settings->cloudlight, settings->cloudlightOscillator);
    CommonViews::ShaderParameter(settings->cloudcover, settings->cloudcoverOscillator);
    CommonViews::ShaderParameter(settings->cloudalpha, settings->cloudalphaOscillator);
    CommonViews::ShaderParameter(settings->skytint, settings->skytintOscillator);
  }
  
  
};

#endif
