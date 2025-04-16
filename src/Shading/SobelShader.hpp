//
//  SobelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SobelShader_hpp
#define SobelShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct SobelSettings : public ShaderSettings {
public:
  std::shared_ptr<Parameter> tolerance;
  std::shared_ptr<WaveformOscillator> toleranceOscillator;
  
  SobelSettings(std::string shaderId, json j, std::string name)
  :   tolerance(std::make_shared<Parameter>("Tolerance", 1.0, 0.0, 5.0)),
  toleranceOscillator(std::make_shared<WaveformOscillator>(tolerance)),
  ShaderSettings(shaderId, j, name){
    parameters = {tolerance};
    oscillators = {toleranceOscillator};
    audioReactiveParameter = tolerance;
    registerParameters();
  };
};

struct SobelShader : Shader {
  SobelSettings *settings;
  SobelShader(SobelSettings *settings) : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/Sobel");
    shader.load("shaders/Sobel");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("tolerance", settings->tolerance->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
  int inputCount() override {
    return 1;
  }
  ShaderType type() override { return ShaderTypeSobel; }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->tolerance, settings->toleranceOscillator);
  }
};

#endif
