//
//  TriangleMapShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TriangleMapShader_hpp
#define TriangleMapShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct TriangleMapSettings: public ShaderSettings {
	public:
  TriangleMapSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

class TriangleMapShader: public Shader {
public:

  TriangleMapSettings *settings;
  TriangleMapShader(TriangleMapSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/TriangleMap");
#endif
#ifdef RELEASE
shader.load("shaders/TriangleMap");
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
    return ShaderTypeTriangleMap;
  }

  void drawSettings() override {
    CommonViews::H3Title("TriangleMap");
  }
};

#endif /* TriangleMapShader_hpp */
