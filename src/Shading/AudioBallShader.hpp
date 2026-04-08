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
  std::shared_ptr<Parameter> dotCount;
  std::shared_ptr<WaveformOscillator> dotCountOscillator;

  std::shared_ptr<Parameter> ringRadius;
  std::shared_ptr<WaveformOscillator> ringRadiusOscillator;

  std::shared_ptr<Parameter> dotBrightness;
  std::shared_ptr<WaveformOscillator> dotBrightnessOscillator;

  std::shared_ptr<Parameter> audioScale;
  std::shared_ptr<WaveformOscillator> audioScaleOscillator;

  std::shared_ptr<Parameter> backgroundColor;

  AudioBallSettings(std::string shaderId, json j) :
  dotCount(std::make_shared<Parameter>("Dot Count", 40.0, 5.0, 80.0, ParameterType_Int)),
  dotCountOscillator(std::make_shared<WaveformOscillator>(dotCount)),
  ringRadius(std::make_shared<Parameter>("Ring Radius", 0.25, 0.05, 0.5)),
  ringRadiusOscillator(std::make_shared<WaveformOscillator>(ringRadius)),
  dotBrightness(std::make_shared<Parameter>("Dot Brightness", 0.02, 0.005, 0.1)),
  dotBrightnessOscillator(std::make_shared<WaveformOscillator>(dotBrightness)),
  audioScale(std::make_shared<Parameter>("Audio Scale", 1.0, 0.0, 5.0)),
  audioScaleOscillator(std::make_shared<WaveformOscillator>(audioScale)),
  backgroundColor(std::make_shared<Parameter>("Background Color", ParameterType_Color)),
  ShaderSettings(shaderId, j, "AudioBall") {
    parameters = { dotCount, ringRadius, dotBrightness, audioScale, backgroundColor };
    oscillators = { dotCountOscillator, ringRadiusOscillator, dotBrightnessOscillator, audioScaleOscillator };
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
    if (source != nullptr)
      shader.setUniform1fv("audio", source->audioAnalysis.renderSpectrum.data(), 256);
    shader.setUniform1f("dotCount", settings->dotCount->value);
    shader.setUniform1f("ringRadius", settings->ringRadius->value);
    shader.setUniform1f("dotBrightness", settings->dotBrightness->value);
    shader.setUniform1f("audioScale", settings->audioScale->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    auto bg = settings->backgroundColor->color->data();
    shader.setUniform4f("bgColor", bg[0], bg[1], bg[2], bg[3]);
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
    CommonViews::ShaderParameter(settings->audioScale, settings->audioScaleOscillator);
    CommonViews::ShaderColor(settings->backgroundColor);
  }
};

#endif /* AudioBallShader_hpp */
