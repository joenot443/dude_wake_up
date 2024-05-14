//
//  PieSplitShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PieSplitShader_hpp
#define PieSplitShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PieSplitSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderWaveformOscillator;

  PieSplitSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 1.0  , -1.0, 2.0)),
  shaderWaveformOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j) {
    parameters = { shaderValue };
    oscillators = { shaderWaveformOscillator };
    load(j);
    registerParameters();
  };
};

struct PieSplitShader: Shader {
  PieSplitSettings *settings;
  PieSplitShader(PieSplitSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  std::set<InputSlot> activeInputs;

  void setup() override {
    shader.load("shaders/PieSplit");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    activeInputs.clear();
    canvas->begin();
    shader.begin();
    
    // Accumulate the connected inputs
    for (auto [slot, conn] : inputs) {
      activeInputs.insert(slot);
      loadTextureAt(slot);
    }
    
    
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void loadTextureAt(InputSlot slot) {
    auto name = "tex" + ofToString(slot + 1);
    auto mixName = "tex" + ofToString(slot + 1) + "_active";
    ofTexture& tex = inputAtSlot(slot)->frame()->getTexture();
    
    int textureUnit = 8 + (int) activeInputs.size() * 4;
    tex.bind(textureUnit);
    shader.setUniformTexture(name, tex, textureUnit);
    shader.setUniform1i(mixName, 1);
  }

  void clear() override {
    
  }
  
  int inputCount() override {
    return std::max(2, std::min(10, static_cast<int>(activeInputs.size() + 1)));
  }
  
  ShaderType type() override {
    return ShaderTypePieSplit;
  }

  void drawSettings() override {
    CommonViews::H3Title("PieSplit");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderWaveformOscillator);
  }
};

#endif /* PieSplitShader_hpp */
