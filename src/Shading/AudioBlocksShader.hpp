//
//  AudioBlocksShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioBlocksShader_hpp
#define AudioBlocksShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "AudioSourceService.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioBlocksSettings: public ShaderSettings {
  std::shared_ptr<Parameter> size;
  std::shared_ptr<WaveformOscillator> sizeOscillator;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<Parameter> ySpread;
  std::shared_ptr<WaveformOscillator> ySpreadOscillator;

  AudioBlocksSettings(std::string shaderId, json j) :
    size(std::make_shared<Parameter>("Size", 1.0, 0.0, 4.0)),
    sizeOscillator(std::make_shared<WaveformOscillator>(size)),
    speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 10.0)),
    speedOscillator(std::make_shared<WaveformOscillator>(speed)),
    ySpread(std::make_shared<Parameter>("Y Spread", 0.5, 0.0, 2.0)),
    ySpreadOscillator(std::make_shared<WaveformOscillator>(ySpread)),
    
    ShaderSettings(shaderId, j, "AudioBlocks") {
    
    parameters = { size, speed, ySpread };
    oscillators = { sizeOscillator, speedOscillator, ySpreadOscillator };
      audioReactiveParameter = size;
    load(j);
    registerParameters();
  };
};

struct AudioBlocksShader: Shader {
  AudioBlocksSettings *settings;
  AudioBlocksShader(AudioBlocksSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/AudioBlocks");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("size", settings->size->value);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("ySpread", settings->ySpread->value);
    shader.setUniform1f("audio", settings->size->value);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0) {
      shader.setUniform1f("audioLow", source->audioAnalysis.lows->value);
      shader.setUniform1f("audioHigh", source->audioAnalysis.highs->value);
    }

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
    return ShaderTypeAudioBlocks;
  }

  void drawSettings() override {

    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->ySpread, settings->ySpreadOscillator);
  }
};
#endif
