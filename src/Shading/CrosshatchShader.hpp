//
//  CrosshatchShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CrosshatchShader_hpp
#define CrosshatchShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CrosshatchSettings: public ShaderSettings {
	public:
  CrosshatchSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

struct CrosshatchShader: Shader {
  CrosshatchSettings *settings;
  CrosshatchShader(CrosshatchSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Crosshatch");
#endif
#ifdef RELEASE
shader.load("shaders/Crosshatch");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofClear(0,0,0,255);
    ofClear(0,0,0,0);
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
    return ShaderTypeCrosshatch;
  }

  void drawSettings() override {
    
    CommonViews::H3Title("Crosshatch");

  }
};

#endif /* CrosshatchShader_hpp */
