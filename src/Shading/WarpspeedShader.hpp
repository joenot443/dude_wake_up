//
//  WarpspeedShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WarpspeedShader_hpp
#define WarpspeedShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WarpspeedSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<ValueOscillator> speedOscillator;

  WarpspeedSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 5.0)),
  speedOscillator(std::make_shared<ValueOscillator>(speed)),
  ShaderSettings(shaderId, j) {
    parameters = { speed };
    oscillators = { speedOscillator };
    load(j);
    registerParameters();
  };
};

struct WarpspeedShader: Shader {
  WarpspeedSettings *settings;
  WarpspeedShader(WarpspeedSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Warpspeed");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("speed", settings->speed->value);
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
    return ShaderTypeWarpspeed;
  }

  void drawSettings() override {
    CommonViews::H3Title("Warpspeed");

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif /* WarpspeedShader_hpp */
