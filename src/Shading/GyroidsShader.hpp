//
//  GyroidsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GyroidsShader_hpp
#define GyroidsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GyroidsSettings: public ShaderSettings {
  
  GyroidsSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name)  {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct GyroidsShader: Shader {
  GyroidsSettings *settings;
  GyroidsShader(GyroidsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Gyroids");
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
    return ShaderTypeGyroids;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Gyroids");
  }
};

#endif /* GyroidsShader_hpp */
