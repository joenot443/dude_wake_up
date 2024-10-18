//
//  AsciiShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AsciiShader_hpp
#define AsciiShader_hpp

#include "ofMain.h"

#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
#include <stdio.h>

// Ascii

struct AsciiSettings: public ShaderSettings  {
  
  std::shared_ptr<Parameter> size;
  std::shared_ptr<WaveformOscillator> sizeOscillator;
  
  AsciiSettings(std::string shaderId, json j, std::string name) :
  size(std::make_shared<Parameter>("Size", 1.0, 0.0, 5.0)),
  sizeOscillator(std::make_shared<WaveformOscillator>(size)),
  ShaderSettings(shaderId, j, name)
  {
    parameters = {size};
    oscillators = {sizeOscillator};
    audioReactiveParameter = size;
    load(j);
    registerParameters();
  }
};


class AsciiShader: public Shader {
public:

  AsciiSettings *settings;
  AsciiShader(AsciiSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
#ifdef DEBUG
    shader.load("shaders/Ascii");
#endif
#ifdef RELEASE
    shader.load("shaders/Ascii");
#endif
  }
  
  int inputCount() override {
    return 1;
  }
  
  ShaderType type() override {
    return ShaderTypeAscii;
  }
  
  bool enabled() override {
    return true;
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("size", settings->size->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
  }
};

#endif /* AsciiShader_hpp */
