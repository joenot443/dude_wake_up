//
//  VHSShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef VHSShader_hpp
#define VHSShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct VHSSettings: public ShaderSettings {
  VHSSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct VHSShader: Shader {
  VHSSettings *settings;
  std::shared_ptr<ofFbo> tempCanvas;
  
  VHSShader(VHSSettings *settings) : settings(settings), Shader(settings), tempCanvas(std::make_shared<ofFbo>()) {};

  ofShader shaderTwo;
  void setup() override {
    
    shader.load("shaders/VHS");
    shaderTwo.load("shaders/VHS2");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    if (!tempCanvas->isAllocated()) {
      tempCanvas->allocate(frame->getWidth(), frame->getHeight());
    }
    
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
    
    
    tempCanvas->begin();
    shaderTwo.begin();
    shaderTwo.setUniformTexture("tex", canvas->getTexture(), 8);
    shaderTwo.setUniform1f("time", ofGetElapsedTimef());
    shaderTwo.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    canvas->draw(0, 0);
    shaderTwo.end();
    tempCanvas->end();
    
    canvas->begin();
    tempCanvas->draw(0, 0);
    canvas->end();
  }

  void clear() override {
    
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeVHS;
  }

  void drawSettings() override {
    CommonViews::H3Title("VHS");
  }
};

#endif
