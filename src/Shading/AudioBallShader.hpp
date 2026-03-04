//
//  AudioBallShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioBallShader_hpp
#define AudioBallShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioBallSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  std::shared_ptr<Parameter> dotCount;
  std::shared_ptr<WaveformOscillator> dotCountOscillator;

  std::shared_ptr<Parameter> ringRadius;
  std::shared_ptr<WaveformOscillator> ringRadiusOscillator;

  std::shared_ptr<Parameter> dotBrightness;
  std::shared_ptr<WaveformOscillator> dotBrightnessOscillator;

  AudioBallSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  dotCount(std::make_shared<Parameter>("Dot Count", 40.0, 5.0, 80.0, ParameterType_Int)),
  dotCountOscillator(std::make_shared<WaveformOscillator>(dotCount)),
  ringRadius(std::make_shared<Parameter>("Ring Radius", 0.25, 0.05, 0.5)),
  ringRadiusOscillator(std::make_shared<WaveformOscillator>(ringRadius)),
  dotBrightness(std::make_shared<Parameter>("Dot Brightness", 0.02, 0.005, 0.1)),
  dotBrightnessOscillator(std::make_shared<WaveformOscillator>(dotBrightness)),
  ShaderSettings(shaderId, j, "AudioBall") {
    parameters = { shaderValue, dotCount, ringRadius, dotBrightness };
    oscillators = { shaderValueOscillator, dotCountOscillator, ringRadiusOscillator, dotBrightnessOscillator };
    load(j);
    registerParameters();
  };
};

struct AudioBallShader: Shader {
  AudioBallSettings *settings;
  AudioBallShader(AudioBallSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/AudioBall");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 254)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("dotCount", settings->dotCount->value);
    shader.setUniform1f("ringRadius", settings->ringRadius->value);
    shader.setUniform1f("dotBrightness", settings->dotBrightness->value);
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
    return ShaderTypeAudioBall;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioBall");
    CommonViews::ShaderIntParameter(settings->dotCount);
    CommonViews::ShaderParameter(settings->ringRadius, settings->ringRadiusOscillator);
    CommonViews::ShaderParameter(settings->dotBrightness, settings->dotBrightnessOscillator);
  }
};

#endif /* AudioBallShader_hpp */
