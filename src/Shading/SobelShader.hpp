//
//  SobelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SobelShader_hpp
#define SobelShader_hpp

#include "CommonViews.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct SobelSettings : public ShaderSettings {
  SobelSettings(std::string shaderId, json j)
      : ShaderSettings(shaderId){

        };
};

struct SobelShader : Shader {
  SobelSettings *settings;
  SobelShader(SobelSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override { shader.load("shaders/Sobel"); }

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

  void clear() override {}

  ShaderType type() override { return ShaderTypeSobel; }

  void drawSettings() override { CommonViews::H3Title("Sobel"); }
};

#endif /* SobelShader_hpp */
