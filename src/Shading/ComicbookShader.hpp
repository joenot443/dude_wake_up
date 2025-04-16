//
//  ComicbookShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ComicbookShader_hpp
#define ComicbookShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ComicbookSettings: public ShaderSettings {
  
  ComicbookSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j, "Comicbook") {
    parameters = { };
    oscillators = {};
    load(j);
    registerParameters();
  };
};

struct ComicbookShader: Shader {
  ComicbookSettings *settings;
  ComicbookShader(ComicbookSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Comicbook");
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
    return ShaderTypeComicbook;
  }

  void drawSettings() override {
    
  }
};

#endif /* ComicbookShader_hpp */
