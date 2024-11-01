//
//  TissueShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TissueShader_hpp
#define TissueShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct TissueSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> modulationSpeed;
  std::shared_ptr<WaveformOscillator> modulationSpeedOscillator;
  std::shared_ptr<Parameter> distortionAmount;
  std::shared_ptr<WaveformOscillator> distortionAmountOscillator;
  std::shared_ptr<Parameter> waveIntensity;
  std::shared_ptr<WaveformOscillator> waveIntensityOscillator;
  std::shared_ptr<Parameter> cycles;
  
  TissueSettings(std::string shaderId, json j, std::string name) :
  modulationSpeed(std::make_shared<Parameter>("Modulation Speed", 1.0, 0.0, 10.0)),
  modulationSpeedOscillator(std::make_shared<WaveformOscillator>(modulationSpeed)),
  distortionAmount(std::make_shared<Parameter>("Distortion Amount", 1.0, 0.8, 2.0)),
  distortionAmountOscillator(std::make_shared<WaveformOscillator>(distortionAmount)),
  waveIntensity(std::make_shared<Parameter>("Wave Intensity", 1.0, -3.14, 3.14)),
  waveIntensityOscillator(std::make_shared<WaveformOscillator>(waveIntensity)),
  cycles(std::make_shared<Parameter>("Cycles", 30, 1, 100)),
  ShaderSettings(shaderId, j, "Tissue") {
    parameters = { modulationSpeed, distortionAmount, waveIntensity, cycles };
    oscillators = { modulationSpeedOscillator, distortionAmountOscillator, waveIntensityOscillator };
    load(j);
    registerParameters();
  };
};

class TissueShader : public Shader {
public:

  TissueSettings *settings;
  TissueShader(TissueSettings *settings)
  : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/Tissue");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    shader.setUniform1f("modulationSpeed", settings->modulationSpeed->value);
    shader.setUniform1f("distortionAmount", settings->distortionAmount->value);
    shader.setUniform1f("waveIntensity", settings->waveIntensity->value);
    shader.setUniform1i("cycles", settings->cycles->intValue);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypeTissue; }
  
  void drawSettings() override {
    
    CommonViews::H3Title("Tissue");
    CommonViews::ShaderParameter(settings->modulationSpeed, settings->modulationSpeedOscillator);
    CommonViews::ShaderParameter(settings->distortionAmount, settings->distortionAmountOscillator);
    CommonViews::ShaderParameter(settings->waveIntensity, settings->waveIntensityOscillator);
    CommonViews::ShaderIntParameter(settings->cycles);
  }
};

#endif
