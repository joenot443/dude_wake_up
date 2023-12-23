//
//  LiquidShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LiquidShader_hpp
#define LiquidShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct LiquidSettings: public ShaderSettings {
	public:
  LiquidSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

class LiquidShader: public Shader {
public:

  LiquidSettings *settings;
  LiquidShader(LiquidSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Liquid");
#endif
#ifdef RELEASE
shader.load("shaders/Liquid");
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

  ShaderType type() override {
    return ShaderTypeLiquid;
  }

  void drawSettings() override {
    
    CommonViews::H3Title("Liquid");
  }
};

#endif /* LiquidShader_hpp */
