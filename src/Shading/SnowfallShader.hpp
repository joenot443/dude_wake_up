//
//  SnowfallShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SnowfallShader_hpp
#define SnowfallShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SnowfallSettings: public ShaderSettings {
  std::shared_ptr<Parameter> flakesCount;
  std::shared_ptr<Parameter> descentSpeed;
  std::shared_ptr<Parameter> size;
  
  std::shared_ptr<WaveformOscillator> flakesCountOscillator;
  std::shared_ptr<WaveformOscillator> descentSpeedOscillator;
  std::shared_ptr<WaveformOscillator> sizeOscillator;
  
  SnowfallSettings(std::string shaderId, json j, std::string name) :
  flakesCount(std::make_shared<Parameter>("flakesCount", 50, 1, 100)),
  descentSpeed(std::make_shared<Parameter>("descentSpeed", 10, 0, 50)),
  size(std::make_shared<Parameter>("size", 0.3, 0.0, 1.0)),
  
  ShaderSettings(shaderId, j, name) {
    flakesCountOscillator = std::make_shared<WaveformOscillator>(flakesCount);
    descentSpeedOscillator = std::make_shared<WaveformOscillator>(descentSpeed);
    sizeOscillator = std::make_shared<WaveformOscillator>(size);
    
    parameters = { flakesCount, descentSpeed, size };
    oscillators = { flakesCountOscillator, descentSpeedOscillator, sizeOscillator };
    load(j);
    registerParameters();
  };
};

struct Flake {
  glm::vec2 position;
  float phase;
  float speed;
};

struct SnowfallShader: Shader {
  SnowfallSettings *settings;
  std::vector<Flake> flakes;

  int flakesCount;
  
  SnowfallShader(SnowfallSettings *settings) : settings(settings), Shader(settings) {
    flakesCount = settings->flakesCount->intValue;
  }
  
  void setup() override {
    shader.load("shaders/Snowfall");
  }
  
  void updateFlakes(float width) {
    if (flakes.size() == 0) {
      createFlakes(width);
    }
    if (flakesCount != settings->flakesCount->intValue) {
      createFlakes(width);
      flakesCount = settings->flakesCount->intValue;
    }
    float time = ofGetElapsedTimef();
    for (auto& flake : flakes) {
      // Update position
      flake.position.y += flake.speed * settings->descentSpeed->value;
      flake.position.x += 10 * sin(0.5*time*flake.phase) * sin(0.2*time*flake.phase) * sin(0.1*time*flake.phase) * sin(0.3*time*flake.phase);
      
      // Reset flake to top if it falls off the bottom
      if (flake.position.y > ofGetHeight()) {
        flake.position.y = ofRandom(-100, 0);
        flake.position.x = ofRandom(ofGetWidth());
        flake.speed = ofRandom(0.5, 1.0);
      }
    }
  }
  
  void createFlakes(float width) {
    flakes.clear();
    for(int i = 0; i < flakesCount; ++i) {
      Flake flake = {
        glm::vec2(ofRandom(width * 0.9), ofRandom(-200, 0)),
        ofRandom(TWO_PI),
        ofRandom(1.0)
      };
      flakes.push_back(flake);
    }
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    
    updateFlakes(frame->getWidth());
    
    canvas->begin();
    
    // Draw each flake
    for (const auto& flake : flakes) {
      frame->draw(flake.position.x,
                  flake.position.y,
                  frame->getWidth() * settings->size->value * 0.5,
                  frame->getHeight() * settings->size->value * 0.5);
    }
    
    canvas->end();
  }
  
  void clear() override {
    flakes.clear();
  }
  
    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeSnowfall;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->flakesCount, settings->flakesCountOscillator);
    CommonViews::ShaderParameter(settings->descentSpeed, settings->descentSpeedOscillator);
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
  }
};


#endif
