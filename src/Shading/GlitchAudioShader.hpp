//
//  GlitchAudioShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GlitchAudioShader_hpp
#define GlitchAudioShader_hpp

#include "ofMain.h"
#include "AudioSourceService.hpp"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GlitchAudioSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  // New Parameters
  std::shared_ptr<Parameter> noiseScale;
  std::shared_ptr<Parameter> noiseXOffset;
  std::shared_ptr<Parameter> noiseYOffset;
  std::shared_ptr<Parameter> audioThreshold;
  std::shared_ptr<Parameter> displacementExponent;
  std::shared_ptr<Parameter> displacementScale;
  std::shared_ptr<Parameter> noiseOffsetR;
  std::shared_ptr<Parameter> noiseOffsetG;
  std::shared_ptr<Parameter> noiseOffsetB;

  // New Oscillators
  std::shared_ptr<WaveformOscillator> noiseScaleOscillator;
  std::shared_ptr<WaveformOscillator> noiseXOffsetOscillator;
  std::shared_ptr<WaveformOscillator> noiseYOffsetOscillator;
  std::shared_ptr<WaveformOscillator> audioThresholdOscillator;
  std::shared_ptr<WaveformOscillator> displacementExponentOscillator;
  std::shared_ptr<WaveformOscillator> displacementScaleOscillator;
  std::shared_ptr<WaveformOscillator> noiseOffsetROscillator;
  std::shared_ptr<WaveformOscillator> noiseOffsetGOscillator;
  std::shared_ptr<WaveformOscillator> noiseOffsetBOscillator;

  GlitchAudioSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  // Initialize new parameters
  noiseScale(std::make_shared<Parameter>("Noise Scale", 2.2, 0.0, 10.0)),
  noiseXOffset(std::make_shared<Parameter>("Noise X Offset", 0.0, -10.0, 10.0)),
  noiseYOffset(std::make_shared<Parameter>("Noise Y Offset", 0.0, -10.0, 10.0)),
  audioThreshold(std::make_shared<Parameter>("Audio Threshold", 0.4, 0.0, 1.0)),
  displacementExponent(std::make_shared<Parameter>("Displacement Exponent", 8.0, 1.0, 16.0)),
  displacementScale(std::make_shared<Parameter>("Displacement Scale", 0.01, 0.0, 0.1)),
  noiseOffsetR(std::make_shared<Parameter>("Noise Offset R", 0.0, 0.0, 50.0)),
  noiseOffsetG(std::make_shared<Parameter>("Noise Offset G", 10.0, 0.0, 50.0)),
  noiseOffsetB(std::make_shared<Parameter>("Noise Offset B", 20.0, 0.0, 50.0)),
  // Initialize new oscillators
  noiseScaleOscillator(std::make_shared<WaveformOscillator>(noiseScale)),
  noiseXOffsetOscillator(std::make_shared<WaveformOscillator>(noiseXOffset)),
  noiseYOffsetOscillator(std::make_shared<WaveformOscillator>(noiseYOffset)),
  audioThresholdOscillator(std::make_shared<WaveformOscillator>(audioThreshold)),
  displacementExponentOscillator(std::make_shared<WaveformOscillator>(displacementExponent)),
  displacementScaleOscillator(std::make_shared<WaveformOscillator>(displacementScale)),
  noiseOffsetROscillator(std::make_shared<WaveformOscillator>(noiseOffsetR)),
  noiseOffsetGOscillator(std::make_shared<WaveformOscillator>(noiseOffsetG)),
  noiseOffsetBOscillator(std::make_shared<WaveformOscillator>(noiseOffsetB)),
  ShaderSettings(shaderId, j, "GlitchAudio") {
    parameters = { shaderValue, noiseScale, noiseXOffset, noiseYOffset, audioThreshold, displacementExponent, displacementScale, noiseOffsetR, noiseOffsetG, noiseOffsetB };
    oscillators = { shaderValueOscillator, noiseScaleOscillator, noiseXOffsetOscillator, noiseYOffsetOscillator, audioThresholdOscillator, displacementExponentOscillator, displacementScaleOscillator, noiseOffsetROscillator, noiseOffsetGOscillator, noiseOffsetBOscillator };
    load(j);
    registerParameters();
  };
};

struct GlitchAudioShader: Shader {
  GlitchAudioSettings *settings;
  GlitchAudioShader(GlitchAudioSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Glitch Audio");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    // Set new uniforms
    shader.setUniform1f("noiseScale", settings->noiseScale->value);
    shader.setUniform1f("noiseXOffset", settings->noiseXOffset->value);
    shader.setUniform1f("noiseYOffset", settings->noiseYOffset->value);
    shader.setUniform1f("audioThreshold", settings->audioThreshold->value);
    shader.setUniform1f("displacementExponent", settings->displacementExponent->value);
    shader.setUniform1f("displacementScale", settings->displacementScale->value);
    shader.setUniform1f("noiseOffsetR", settings->noiseOffsetR->value);
    shader.setUniform1f("noiseOffsetG", settings->noiseOffsetG->value);
    shader.setUniform1f("noiseOffsetB", settings->noiseOffsetB->value);

    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
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
    return ShaderTypeGlitchAudio;
  }

  void drawSettings() override {
    

    
    // Draw new parameter controls
    CommonViews::ShaderParameter(settings->noiseScale, settings->noiseScaleOscillator);
    CommonViews::ShaderParameter(settings->noiseXOffset, settings->noiseXOffsetOscillator);
    CommonViews::ShaderParameter(settings->noiseYOffset, settings->noiseYOffsetOscillator);
    CommonViews::ShaderParameter(settings->audioThreshold, settings->audioThresholdOscillator);
    CommonViews::ShaderParameter(settings->displacementExponent, settings->displacementExponentOscillator);
    CommonViews::ShaderParameter(settings->displacementScale, settings->displacementScaleOscillator);
    CommonViews::ShaderParameter(settings->noiseOffsetR, settings->noiseOffsetROscillator);
    CommonViews::ShaderParameter(settings->noiseOffsetG, settings->noiseOffsetGOscillator);
    CommonViews::ShaderParameter(settings->noiseOffsetB, settings->noiseOffsetBOscillator);
  }
};

#endif
