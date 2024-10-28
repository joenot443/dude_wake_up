//
//  StarryPlanesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef StarryPlanesShader_hpp
#define StarryPlanesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct StarryPlanesSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> speedOscillator;
  std::shared_ptr<Parameter> rotation;
  std::shared_ptr<WaveformOscillator> rotationOscillator;
  std::shared_ptr<Parameter> planeDist;
  std::shared_ptr<WaveformOscillator> planeDistOscillator;
  std::shared_ptr<Parameter> count;
  std::shared_ptr<WaveformOscillator> countOscillator;

  StarryPlanesSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", 1.0, -1.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  rotation(std::make_shared<Parameter>("rotation", 1.0, -1.0, 1.0)),
  rotationOscillator(std::make_shared<WaveformOscillator>(speed)),
  planeDist(std::make_shared<Parameter>("planeDist", 1.0, 0.0, 5.0)),
  planeDistOscillator(std::make_shared<WaveformOscillator>(speed)),
  count(std::make_shared<Parameter>("count", 1.0, 0.0, 5.0)),
  countOscillator(std::make_shared<WaveformOscillator>(speed)),
  
  
  ShaderSettings(shaderId, j, "StarryPlanes") {
    parameters = { speed, rotation, planeDist, count };
    oscillators = { speedOscillator, rotationOscillator, planeDistOscillator, countOscillator };
    load(j);
    registerParameters();
  };
};

struct StarryPlanesShader: Shader {
  StarryPlanesSettings *settings;
  StarryPlanesShader(StarryPlanesSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/Empty");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("rotation", settings->rotation->value);
    shader.setUniform1f("planeDist", settings->planeDist->value);
    shader.setUniform1f("count", settings->count->value);
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
    return ShaderTypeStarryPlanes;
  }

  void drawSettings() override {
    CommonViews::H3Title("StarryPlanes");

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif
