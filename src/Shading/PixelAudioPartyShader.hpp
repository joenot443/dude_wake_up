//
//  PixelAudioPartyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelAudioPartyShader_hpp
#define PixelAudioPartyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PixelAudioPartySettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;

  PixelAudioPartySettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("amount", 0.5, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, "PixelAudioParty") {
    parameters = { amount };
    oscillators = { amountOscillator };
    load(j);
    registerParameters();
  };
};

struct PixelAudioPartyShader: Shader {
  PixelAudioPartySettings *settings;
  PixelAudioPartyShader(PixelAudioPartySettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/PixelAudioParty");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);

    shader.setUniform1f("time", ofGetElapsedTimef());
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
    return ShaderTypePixelAudioParty;
  }

  void drawSettings() override {
    CommonViews::H3Title("PixelAudioParty");

    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* PixelAudioPartyShader_hpp */
