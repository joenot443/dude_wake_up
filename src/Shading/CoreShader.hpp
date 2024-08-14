//
//  CoreShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CoreShader_hpp
#define CoreShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CoreSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shapeDistance;
  std::shared_ptr<ValueOscillator> shapeDistanceOscillator;
  
  //Speed
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<ValueOscillator> speedOscillator;

  CoreSettings(std::string shaderId, json j) :
  shapeDistance(std::make_shared<Parameter>("Shape Distance", 0.5, 0.0, 1.0)),
  shapeDistanceOscillator(std::make_shared<ValueOscillator>(shapeDistance)),
  speed(std::make_shared<Parameter>("Speed", 0.5, 0.0, 1.0)),
  speedOscillator(std::make_shared<ValueOscillator>(speed)),
  ShaderSettings(shaderId, j, "Core") {
    parameters = { shapeDistance };
    oscillators = { shapeDistanceOscillator };
    load(j);
    registerParameters();
  };
};

struct CoreShader: Shader {
  CoreSettings *settings;
  CoreShader(CoreSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Core");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shapeDistance", settings->shapeDistance->value);
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
    return ShaderTypeCore;
  }

  void drawSettings() override {
    CommonViews::H3Title("Core");

    CommonViews::ShaderParameter(settings->shapeDistance, settings->shapeDistanceOscillator);
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif /* CoreShader_hpp */
