//
//  MelterShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/2/22.
//

#ifndef MelterShader_h
#define MelterShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MeltSettings : public ShaderSettings {
	public:
  std::string shaderId;

  MeltSettings(std::string shaderId, json j) : shaderId(shaderId),
  ShaderSettings(shaderId, j) {
    load(j);
  registerParameters();
  };
};

class MeltShader : public Shader {
public:

  private:
  ofShader shader;

  public:
  MeltSettings *settings;

  MeltShader(MeltSettings *settings) : settings(settings),
  Shader(settings) {
    #ifdef TESTING
shader.load("shaders/Melt");
#endif
#ifdef RELEASE
shader.load("shaders/Melt");
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

  void drawSettings() override {
    
    ImGui::Text("Melt");
  }
};
#endif /* MelterShader_h */
