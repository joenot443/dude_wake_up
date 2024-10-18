//
//  GalaxyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GalaxyShader_hpp
#define GalaxyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GalaxySettings: public ShaderSettings {
	public:
  GalaxySettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class GalaxyShader: public Shader {
public:

  GalaxySettings *settings;
  GalaxyShader(GalaxySettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef DEBUG
shader.load("shaders/Galaxy");
#endif
#ifdef RELEASE
shader.load("shaders/Galaxy");
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
    return ShaderTypeGalaxy;
  }

  void drawSettings() override {
  }
};

#endif /* GalaxyShader_hpp */
