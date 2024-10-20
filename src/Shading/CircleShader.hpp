//
//  CircleShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CircleShader_hpp
#define CircleShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct CircleSettings: public ShaderSettings {
	public:
  CircleSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, 0, "Circle") {
    
  };
};

struct CircleShader: public Shader {
  CircleSettings *settings;
  CircleShader(CircleSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
#ifdef DEBUG
shader.load("shaders/Circle");
#endif
#ifdef RELEASE
shader.load("shaders/Circle");
#endif
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
    return ShaderTypeCircle;
  }

  void drawSettings() override {
    CommonViews::H3Title("Circle");

  }
};

#endif /* CircleShader_hpp */
