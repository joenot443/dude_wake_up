//
//  PsycurvesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PsycurvesShader_hpp
#define PsycurvesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct PsycurvesSettings: public ShaderSettings {
  PsycurvesSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct PsycurvesShader: Shader {
  PsycurvesSettings *settings;
  PsycurvesShader(PsycurvesSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Psycurves");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
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

  ShaderType type() override {
    return ShaderTypePsycurves;
  }

  void drawSettings() override {
    CommonViews::H3Title("Psycurves");

  }
};

#endif /* PsycurvesShader_hpp */
