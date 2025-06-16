//
//  VHSShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef VHSShader_hpp
#define VHSShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct VHSSettings: public ShaderSettings {
  std::shared_ptr<Parameter> hsyncJitter;
  std::shared_ptr<WaveformOscillator> hsyncJitterOscillator;
  
  // VHS.frag parameters
  std::shared_ptr<Parameter> chromaEmphasis;
  std::shared_ptr<WaveformOscillator> chromaEmphasisOscillator;
  std::shared_ptr<Parameter> noiseIntensity;
  std::shared_ptr<WaveformOscillator> noiseIntensityOscillator;
  std::shared_ptr<Parameter> noiseFrequency;
  std::shared_ptr<WaveformOscillator> noiseFrequencyOscillator;
  std::shared_ptr<Parameter> crossColorIntensity;
  std::shared_ptr<WaveformOscillator> crossColorIntensityOscillator;
  
  // VHS2.frag parameters
  std::shared_ptr<Parameter> blurIntensity;
  std::shared_ptr<WaveformOscillator> blurIntensityOscillator;
  std::shared_ptr<Parameter> jitterIntensity;
  std::shared_ptr<WaveformOscillator> jitterIntensityOscillator;
  std::shared_ptr<Parameter> jitterFrequency;
  std::shared_ptr<WaveformOscillator> jitterFrequencyOscillator;
  std::shared_ptr<Parameter> kernelBias;
  std::shared_ptr<WaveformOscillator> kernelBiasOscillator;

  VHSSettings(std::string shaderId, json j, std::string name) :
  hsyncJitter(std::make_shared<Parameter>("Hsync Jitter", 0.5, 1.0, 0.0)),
  hsyncJitterOscillator(std::make_shared<WaveformOscillator>(hsyncJitter)),
  chromaEmphasis(std::make_shared<Parameter>("Chroma Emphasis", 0.5, 0.0, 2.0)),
  chromaEmphasisOscillator(std::make_shared<WaveformOscillator>(chromaEmphasis)),
  noiseIntensity(std::make_shared<Parameter>("Noise Intensity", 1.0, 0.0, 1.0)),
  noiseIntensityOscillator(std::make_shared<WaveformOscillator>(noiseIntensity)),
  noiseFrequency(std::make_shared<Parameter>("Noise Frequency", 1.0, 1.0, 2.0)),
  noiseFrequencyOscillator(std::make_shared<WaveformOscillator>(noiseFrequency)),
  crossColorIntensity(std::make_shared<Parameter>("Cross Color Intensity", 1.0, 0.0, 2.0)),
  crossColorIntensityOscillator(std::make_shared<WaveformOscillator>(crossColorIntensity)),
  blurIntensity(std::make_shared<Parameter>("Blur Intensity", 1.0, 0.0, 2.0)),
  blurIntensityOscillator(std::make_shared<WaveformOscillator>(blurIntensity)),
  jitterIntensity(std::make_shared<Parameter>("Jitter Intensity", 1.0, 0.0, 3.0)),
  jitterIntensityOscillator(std::make_shared<WaveformOscillator>(jitterIntensity)),
  jitterFrequency(std::make_shared<Parameter>("Jitter Frequency", 1.0, 0.5, 2.0)),
  jitterFrequencyOscillator(std::make_shared<WaveformOscillator>(jitterFrequency)),
  kernelBias(std::make_shared<Parameter>("Kernel Bias", 0.0, -1.0, 1.0)),
  kernelBiasOscillator(std::make_shared<WaveformOscillator>(kernelBias)),

  ShaderSettings(shaderId, j, name) {
    // Initialize parameters with default values
    parameters = { 
      hsyncJitter, chromaEmphasis, noiseIntensity, noiseFrequency, 
      crossColorIntensity, blurIntensity, jitterIntensity, 
      jitterFrequency, kernelBias 
    };
    
    oscillators = { 
      hsyncJitterOscillator, chromaEmphasisOscillator, noiseIntensityOscillator,
      noiseFrequencyOscillator, crossColorIntensityOscillator, blurIntensityOscillator,
      jitterIntensityOscillator, jitterFrequencyOscillator, kernelBiasOscillator
    };
    
    load(j);
    registerParameters();
  };
};

struct VHSShader: Shader {
  VHSSettings *settings;
  std::shared_ptr<ofFbo> tempCanvas;
  
  VHSShader(VHSSettings *settings) : settings(settings), Shader(settings), tempCanvas(std::make_shared<ofFbo>()) {};

  ofShader shaderTwo;
  void setup() override {
    shader.load("shaders/VHS");
    shaderTwo.load("shaders/VHS2");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    if (!tempCanvas->isAllocated()) {
      tempCanvas->allocate(frame->getWidth(), frame->getHeight());
    }
    
    // First pass - VHS.frag
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("chromaEmphasis", settings->chromaEmphasis->value);
    shader.setUniform1f("noiseIntensity", settings->noiseIntensity->value);
    shader.setUniform1f("noiseFrequency", settings->noiseFrequency->value);
    shader.setUniform1f("crossColorIntensity", settings->crossColorIntensity->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
    
    // Second pass - VHS2.frag
    tempCanvas->begin();
    shaderTwo.begin();
    shaderTwo.setUniformTexture("tex", canvas->getTexture(), 8);
    shaderTwo.setUniform1f("time", ofGetElapsedTimef());
    shaderTwo.setUniform1f("hsyncJitter", settings->hsyncJitter->value);
    shaderTwo.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shaderTwo.setUniform1f("blurIntensity", settings->blurIntensity->value);
    shaderTwo.setUniform1f("jitterIntensity", settings->jitterIntensity->value);
    shaderTwo.setUniform1f("jitterFrequency", settings->jitterFrequency->value);
    shaderTwo.setUniform1f("kernelBias", settings->kernelBias->value);
    canvas->draw(0, 0);
    shaderTwo.end();
    tempCanvas->end();
    
    canvas->begin();
    tempCanvas->draw(0, 0);
    canvas->end();
  }

  void clear() override {
  }

  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeVHS;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->hsyncJitter, settings->hsyncJitterOscillator);
    CommonViews::ShaderParameter(settings->chromaEmphasis, settings->chromaEmphasisOscillator);
    CommonViews::ShaderParameter(settings->noiseIntensity, settings->noiseIntensityOscillator);
    CommonViews::ShaderParameter(settings->noiseFrequency, settings->noiseFrequencyOscillator);
    CommonViews::ShaderParameter(settings->crossColorIntensity, settings->crossColorIntensityOscillator);
    CommonViews::ShaderParameter(settings->blurIntensity, settings->blurIntensityOscillator);
    CommonViews::ShaderParameter(settings->jitterIntensity, settings->jitterIntensityOscillator);
    CommonViews::ShaderParameter(settings->jitterFrequency, settings->jitterFrequencyOscillator);
    CommonViews::ShaderParameter(settings->kernelBias, settings->kernelBiasOscillator);
  }
};

#endif
