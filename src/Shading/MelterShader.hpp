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
  std::string shaderId;

  MeltSettings(std::string shaderId, json j) : shaderId(shaderId),
  ShaderSettings(shaderId, j) {
    load(j);
  registerParameters();
  };
};

struct MeltShader : public Shader {
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

  void drawSettings() override {
    ShaderConfigSelectionView::draw(this);
    ImGui::Text("Melt");
  }
};
#endif /* MelterShader_h */
