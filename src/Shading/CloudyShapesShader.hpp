//
//  CloudyShapesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CloudyShapesShader_hpp
#define CloudyShapesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CloudyShapesSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;
  std::shared_ptr<WaveformOscillator> speedOscillator;

  CloudyShapesSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  ShaderSettings(shaderId, j, "CloudyShapes") {
    parameters = { shaderValue, speed };
    oscillators = { shaderValueOscillator, speedOscillator };
    load(j);
    registerParameters();
  };
};

struct CloudyShapesShader: Shader {
  CloudyShapesSettings *settings;
  CloudyShapesShader(CloudyShapesSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  
  void setup() override {
    shader.load("shaders/CloudyShapes");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
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
    return ShaderTypeCloudyShapes;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);

    
  }
};

#endif
