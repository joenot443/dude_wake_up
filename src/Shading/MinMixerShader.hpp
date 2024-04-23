//
//  MinMixerShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MinMixerShader_hpp
#define MinMixerShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MinMixerSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderWaveformOscillator;

  MinMixerSettings(std::string shaderId, json j, std::string name) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 1.0  , -1.0, 2.0)),
  shaderWaveformOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, name) {
    parameters = { shaderValue };
    oscillators = { shaderWaveformOscillator };
    load(j);
    registerParameters();
  };
};

struct MinMixerShader: Shader {
  MinMixerSettings *settings;
  MinMixerShader(MinMixerSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/MinMixer");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    ofEnableAlphaBlending();
    canvas->begin();
    shader.begin();
    if (auxConnected()) {
      std::shared_ptr<ofFbo> tex2 = aux()->frame();
      shader.setUniformTexture("tex2", tex2->getTexture(), 8);
    } else {
      ofClear(0,0,0,255);
      ofClear(0,0,0,0);
    }
    
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeMinMixer;
  }

  void drawSettings() override {
    CommonViews::H3Title("MinMixer");
  }
};

#endif /* MinMixerShader_hpp */
