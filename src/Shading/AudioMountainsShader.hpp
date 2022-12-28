//
//  AudioMountainsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioMountainsShader_hpp
#define AudioMountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct AudioMountainsSettings: public ShaderSettings {
  AudioMountainsSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId) {
    
  };
};

struct AudioMountainsShader: Shader {
  AudioMountainsSettings *settings;
  AudioMountainsShader(AudioMountainsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("../../shaders/AudioMountains");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeAudioMountains;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioMountains");

  }
};

#endif /* AudioMountainsShader_hpp */
