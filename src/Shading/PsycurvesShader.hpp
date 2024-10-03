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
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PsycurvesSettings: public ShaderSettings {
	public:
  PsycurvesSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class PsycurvesShader: public Shader {
public:

  PsycurvesSettings *settings;
  PsycurvesShader(PsycurvesSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef TESTING
shader.load("shaders/Psycurves");
#endif
#ifdef RELEASE
shader.load("shaders/Psycurves");
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
    return ShaderTypePsycurves;
  }

  void drawSettings() override {
    
    CommonViews::H3Title("Psycurves");

  }
};

#endif /* PsycurvesShader_hpp */
