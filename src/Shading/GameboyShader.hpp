//
//  GameboyShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GameboyShader_hpp
#define GameboyShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "ImageTexture.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct GameboySettings: public ShaderSettings {

  GameboySettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct GameboyShader: Shader {
  GameboySettings *settings;
  ImageTexture noiseTexture;

  GameboyShader(GameboySettings *settings) : 
  settings(settings),
  noiseTexture(nullptr),
  Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Gameboy");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
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
    return ShaderTypeGameboy;
  }

  void drawSettings() override {
    CommonViews::H3Title("Gameboy");
  }
};

#endif /* GameboyShader_hpp */
