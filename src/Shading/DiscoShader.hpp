//
//  DiscoShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DiscoShader_hpp
#define DiscoShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct DiscoSettings: public ShaderSettings {
	public:
  DiscoSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class DiscoShader: public Shader {
public:

  DiscoSettings *settings;
  DiscoShader(DiscoSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
shader.load("shaders/Disco");
shader.load("shaders/Disco");
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
    return ShaderTypeDisco;
  }

  void drawSettings() override {
    

  }
};

#endif
