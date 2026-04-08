//
//  AudioGlowBarsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioGlowBarsShader_hpp
#define AudioGlowBarsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include "AudioSourceService.hpp"
#include <stdio.h>

struct AudioGlowBarsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> barHeight;
  std::shared_ptr<WaveformOscillator> barHeightOscillator;

  std::shared_ptr<Parameter> bloomIntensity;
  std::shared_ptr<WaveformOscillator> bloomIntensityOscillator;

  std::shared_ptr<Parameter> bloomSize;
  std::shared_ptr<WaveformOscillator> bloomSizeOscillator;

  std::shared_ptr<Parameter> backgroundColor;

  std::shared_ptr<Parameter> boost;
  std::shared_ptr<WaveformOscillator> boostOscillator;

  AudioGlowBarsSettings(std::string shaderId, json j) :
  barHeight(std::make_shared<Parameter>("Bar Height", 0.7, 0.1, 1.0)),
  barHeightOscillator(std::make_shared<WaveformOscillator>(barHeight)),
  bloomIntensity(std::make_shared<Parameter>("Bloom Intensity", 0.3, 0.0, 1.0)),
  bloomIntensityOscillator(std::make_shared<WaveformOscillator>(bloomIntensity)),
  bloomSize(std::make_shared<Parameter>("Bloom Size", 12.0, 1.0, 30.0)),
  bloomSizeOscillator(std::make_shared<WaveformOscillator>(bloomSize)),
  backgroundColor(std::make_shared<Parameter>("Background Color", ParameterType_Color)),
  boost(std::make_shared<Parameter>("Boost", 3.0, 1.0, 10.0)),
  boostOscillator(std::make_shared<WaveformOscillator>(boost)),
  ShaderSettings(shaderId, j, "AudioGlowBars") {
    parameters = { barHeight, bloomIntensity, bloomSize, backgroundColor, boost };
    oscillators = { barHeightOscillator, bloomIntensityOscillator, bloomSizeOscillator, boostOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioGlowBarsShader: Shader {
  AudioGlowBarsSettings *settings;
  AudioGlowBarsShader(AudioGlowBarsSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/AudioGlowBars");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    // Clear canvas to ensure alpha channel is properly set
    ofClear(0, 0, 0, 0);
    shader.begin();

    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);

    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("barHeight", settings->barHeight->value);
    shader.setUniform1f("bloomIntensity", settings->bloomIntensity->value);
    shader.setUniform1f("bloomSize", settings->bloomSize->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    auto bg = settings->backgroundColor->color->data();
    shader.setUniform4f("bgColor", bg[0], bg[1], bg[2], bg[3]);
    shader.setUniform1f("boost", settings->boost->value);
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
    return ShaderTypeAudioGlowBars;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioGlowBars");
    CommonViews::ShaderParameter(settings->barHeight, settings->barHeightOscillator);
    CommonViews::ShaderParameter(settings->bloomIntensity, settings->bloomIntensityOscillator);
    CommonViews::ShaderParameter(settings->bloomSize, settings->bloomSizeOscillator);
    CommonViews::ShaderColor(settings->backgroundColor);
    CommonViews::ShaderParameter(settings->boost, settings->boostOscillator);
  }
};

#endif /* AudioGlowBarsShader_hpp */
