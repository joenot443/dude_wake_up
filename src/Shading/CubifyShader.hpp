//
//  CubifyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CubifyShader_hpp
#define CubifyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CubifySettings: public ShaderSettings {
  std::shared_ptr<Parameter> cubeSize;
  std::shared_ptr<ValueOscillator> cubeSizeOscillator;

  CubifySettings(std::string shaderId, json j, std::string name) :
  cubeSize(std::make_shared<Parameter>("cubeSize", 1.0, 0.0, 10.0)),
  cubeSizeOscillator(std::make_shared<ValueOscillator>(cubeSize)),
  ShaderSettings(shaderId, j, name)  {
    parameters = { cubeSize };
    oscillators = { cubeSizeOscillator };
    load(j);
    registerParameters();
  };
};

struct CubifyShader: Shader {
  CubifySettings *settings;
  CubifyShader(CubifySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Cubify");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("cubeSize", settings->cubeSize->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeCubify;
  }

  void drawSettings() override {
    CommonViews::H3Title("Cubify");

    CommonViews::ShaderParameter(settings->cubeSize, settings->cubeSizeOscillator);
  }
};

#endif /* CubifyShader_hpp */