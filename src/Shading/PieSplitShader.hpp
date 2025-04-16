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
  
  PieSplitSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "PieSplit") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct PieSplitShader: Shader {
  PieSplitSettings *settings;
  PieSplitShader(PieSplitSettings *settings) : settings(settings), Shader(settings) {};

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
    
  }
};

#endif
