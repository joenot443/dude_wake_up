//
//  MountainsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MountainsShader_hpp
#define MountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MountainsSettings: public ShaderSettings {
	public:
  MountainsSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

class MountainsShader: public Shader {
public:

  MountainsSettings *settings;
  MountainsShader(MountainsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Mountains");
#endif
#ifdef RELEASE
shader.load("shaders/Mountains");
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
    return ShaderTypeMountains;
  }

  void drawSettings() override {
  }
};

#endif /* MountainsShader_hpp */
