//
//  SobelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SobelShader_hpp
#define SobelShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct SobelSettings : public ShaderSettings {
	public:
  SobelSettings(std::string shaderId, json j)
      : ShaderSettings(shaderId, j){

        };
};

struct SobelShader : Shader {
  SobelSettings *settings;
  SobelShader(SobelSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override {
#ifdef TESTING
shader.load("shaders/Sobel");
#endif
#ifdef RELEASE
shader.load("shaders/Sobel");
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

  void clear() override {}

  ShaderType type() override { return ShaderTypeSobel; }

  void drawSettings() override {
     CommonViews::H3Title("Sobel"); }
};

#endif /* SobelShader_hpp */
