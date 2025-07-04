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
  std::shared_ptr<Parameter> width;
  std::shared_ptr<WaveformOscillator> toleranceOscillator;
  std::shared_ptr<WaveformOscillator> widthOscillator;
  
  SobelSettings(std::string shaderId, json j, std::string name)
  :   tolerance(std::make_shared<Parameter>("Tolerance", 1.0, 0.0, 5.0)),
      width(std::make_shared<Parameter>("Width", 1.0, 1.0, 10.0)),
      toleranceOscillator(std::make_shared<WaveformOscillator>(tolerance)),
      widthOscillator(std::make_shared<WaveformOscillator>(width)),
      ShaderSettings(shaderId, j, name){
    parameters = {tolerance, width};
    oscillators = {toleranceOscillator, widthOscillator};
    audioReactiveParameter = tolerance;
    registerParameters();
  };
};

struct SobelShader : Shader {
  SobelSettings *settings;
  SobelShader(SobelSettings *settings) : settings(settings), Shader(settings){};

  void setup() override {
    shader.load("shaders/Sobel");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1f("tolerance", settings->tolerance->value);
    shader.setUniform1f("width", settings->width->value);
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
    CommonViews::ShaderParameter(settings->width, settings->widthOscillator);
  }
};

#endif
