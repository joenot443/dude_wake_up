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
  DancingSquaresSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    parameters = {};
    oscillators = {};
    load(j);
    registerParameters();
  };
};

struct DancingSquaresShader: Shader {
  DancingSquaresSettings *settings;
  DancingSquaresShader(DancingSquaresSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/DancingSquares");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform1f("time", ofGetElapsedTimef());
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

  ShaderType type() override {
    return ShaderTypeDancingSquares;
  }

  void drawSettings() override {
    CommonViews::H3Title("DancingSquares");

  }
};

#endif /* DancingSquaresShader_hpp */
