//
//  MixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/13/22.
//

#ifndef MixShader_h
#define MixShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "VideoSourceService.hpp"
#include "Oscillator.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MixSettings : public ShaderSettings {
  std::shared_ptr<Parameter> mix;
  std::string mixSourceId = "";
  std::shared_ptr<Oscillator> mixOscillator;

  MixSettings(std::string shaderId, json j) :
  mixSourceId(mixSourceId),
  mix(std::make_shared<Parameter>("mix", shaderId, 0.5, 0.0, 1.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId) {
    parameters = {mix};
    oscillators = {mixOscillator};
    load(j);
  };
};

struct MixShader : public Shader {
  ofShader shader;
  MixSettings *settings;

  MixShader(MixSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Mix");
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    
    if (aux != nullptr && aux->feedbackDestination() != nullptr) {
      ofTexture feedbackTexture = aux->feedbackDestination()->getFrame(0);
      shader.setUniformTexture("tex2", feedbackTexture, 5);
    }
    
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("tex2_mix", settings->mix->value);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderType::ShaderTypeMix;
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
  }
  
};

#endif /* MixShader_h */
