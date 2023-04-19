//
//  OctahedronShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OctahedronShader_hpp
#define OctahedronShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OctahedronSettings: public ShaderSettings {
  OctahedronSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

struct OctahedronShader: Shader {
  OctahedronSettings *settings;
  OctahedronShader(OctahedronSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Octahedron");
#endif
#ifdef RELEASE
shader.load("shaders/Octahedron");
#endif
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
    return ShaderTypeOctahedron;
  }

  void drawSettings() override {
    CommonViews::H3Title("Octahedron");

  }
};

#endif /* OctahedronShader_hpp */
