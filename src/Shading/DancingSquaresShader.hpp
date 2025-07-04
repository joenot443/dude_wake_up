//
//  DancingSquaresShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DancingSquaresShader_hpp
#define DancingSquaresShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "AudioSourceService.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct DancingSquaresSettings: public ShaderSettings {
public:
  DancingSquaresSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    parameters = {};
    oscillators = {};
    load(j);
    registerParameters();
  };
};

class DancingSquaresShader: public Shader {
public:
  DancingSquaresSettings *settings;
  DancingSquaresShader(DancingSquaresSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Dancing Squares");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    auto source = AudioSourceService::getService()->selectedAudioSource;
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0], 256);
    
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
    return ShaderTypeDancingSquares;
  }
  
  void drawSettings() override {
    
  }
};

#endif
