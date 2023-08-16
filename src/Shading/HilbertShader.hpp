//
//  HilbertShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HilbertShader_hpp
#define HilbertShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct HilbertSettings: public ShaderSettings {
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<ValueOscillator> shaderValueOscillator;

  HilbertSettings(std::string shaderId, json j) :
  shaderValue(std::make_shared<Parameter>("shaderValue", 1.0  , -1.0, 2.0)),
  shaderValueOscillator(std::make_shared<ValueOscillator>(shaderValue)),
  ShaderSettings(shaderId, j) {
    parameters = { shaderValue };
    oscillators = { shaderValueOscillator };
    load(j);
    registerParameters();
  };
};

struct HilbertShader: Shader {
  HilbertSettings *settings;
  HilbertShader(HilbertSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Hilbert");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeHilbert;
  }

  void drawSettings() override {
    CommonViews::H3Title("Hilbert");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderValueOscillator);
  }
};

#endif /* HilbertShader_hpp */
