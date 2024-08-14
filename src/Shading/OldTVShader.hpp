//
//  OldTVShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OldTVShader_hpp
#define OldTVShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OldTVSettings: public ShaderSettings {
  OldTVSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "OldTV") {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct OldTVShader: Shader {
  OldTVSettings *settings;
  OldTVShader(OldTVSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/OldTV");
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
    return ShaderTypeOldTV;
  }

  void drawSettings() override {
    CommonViews::H3Title("OldTV");
  }
};

#endif /* OldTVShader_hpp */
