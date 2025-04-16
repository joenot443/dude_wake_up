//
//  VanGoghShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef VanGoghShader_hpp
#define VanGoghShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct VanGoghSettings: public ShaderSettings {
	public:
  VanGoghSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class VanGoghShader: public Shader {
public:

  VanGoghSettings *settings;
  VanGoghShader(VanGoghSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
shader.load("shaders/VanGogh");
shader.load("shaders/VanGogh");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
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
    return ShaderTypeVanGogh;
  }

  void drawSettings() override {
    

  }
};

#endif
