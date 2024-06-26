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
#include <stdio.h>

// Ascii

struct AsciiSettings: public ShaderSettings  {
  
  AsciiSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name)
  {
    parameters = {};
    oscillators = {};
    load(j);
  registerParameters();
  }
};


class AsciiShader: public Shader {
public:
  ofShader shader;
  AsciiSettings *settings;
  AsciiShader(AsciiSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    #ifdef TESTING
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
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    
  }
};

#endif /* AsciiShader_hpp */
