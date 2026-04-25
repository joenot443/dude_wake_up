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
  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<Parameter> autoRotate;

  std::shared_ptr<WaveformOscillator> rotationOscillator;
  std::shared_ptr<WaveformOscillator> autoRotateOscillator;

  PieSplitSettings(std::string shaderId, json j) :
  rotation(std::make_shared<Parameter>("Rotation", 0.0, -360.0, 360.0)),
  autoRotate(std::make_shared<Parameter>("Auto-Rotate", 0.0, -2.0, 2.0)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
  ShaderSettings(shaderId, j, "PieSplit") {
    parameters = { rotation, autoRotate };
    oscillators = { rotationOscillator, autoRotateOscillator };
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
    
    float rotationDegrees = settings->rotation->value;
    if (std::abs(settings->autoRotate->value) > 0.0001) {
      rotationDegrees += ofGetElapsedTimef() * settings->autoRotate->value * 50.0;
    }

    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("rotation", rotationDegrees);
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
    int needed = 1;
    for (auto const& [slot, _] : inputs) {
      needed = std::max(needed, static_cast<int>(slot) + 2);
    }
    return std::max(2, std::min(10, needed));
  }
  
  ShaderType type() override {
    return ShaderTypePieSplit;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
    CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
  }
};

#endif
