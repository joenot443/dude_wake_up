//
//  BlurryTrailShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlurryTrailShader_hpp
#define BlurryTrailShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BlurryTrailSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderValueOscillator;

  BlurryTrailSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 0.5, 0.0, 1.0)),
  shaderValueOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
  ShaderSettings(shaderId, j, "BlurryTrail") {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct BlurryTrailShader: Shader {
  BlurryTrailSettings *settings;
  BlurryTrailShader(BlurryTrailSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/BlurryTrail");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shaderValue", settings->shaderValue->value);
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
    return ShaderTypeBlurryTrail;
  }

  void drawSettings() override {
    

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif /* BlurryTrailShader_hpp */
