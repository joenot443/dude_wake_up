//
//  EmptyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef EmptyShader_hpp
#define EmptyShader_hpp

#include "ofMain.h"

#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct EmptySettings: public ShaderSettings {
	public:
  EmptySettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class EmptyShader: public Shader {
public:

  EmptySettings *settings;
  EmptyShader(EmptySettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/empty");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  int inputCount() override {
    return 0;
  }

  void clear() override {
    
  }
};

#endif /* EmptyShader_hpp */
