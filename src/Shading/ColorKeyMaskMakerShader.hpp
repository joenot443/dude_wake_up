//
//  ColorKeyMaskMakerShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorKeyMaskMakerShader_hpp
#define ColorKeyMaskMakerShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ColorKeyMaskMakerSettings: public ShaderSettings {
  std::shared_ptr<Parameter> tolerance;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<ValueOscillator> toleranceOscillator;

  ColorKeyMaskMakerSettings(std::string shaderId, json j) :
  tolerance(std::make_shared<Parameter>("tolerance", 0.1, 0.0, 1.0)),
  color(std::make_shared<Parameter>("color", 0.0)),
  toleranceOscillator(std::make_shared<ValueOscillator>(tolerance)),
  ShaderSettings(shaderId, j) {
    parameters = { tolerance, color };
    oscillators = { toleranceOscillator };
    load(j);
    registerParameters();
  };
};

struct ColorKeyMaskMakerShader: Shader {
  ColorKeyMaskMakerSettings *settings;
  ColorKeyMaskMakerShader(ColorKeyMaskMakerSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/ColorKeyMaskMaker");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("tolerance", settings->tolerance->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("chromaKey", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], 1.0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeColorKeyMaskMaker;
  }

  void drawSettings() override {
    CommonViews::H3Title("ColorKeyMaskMaker");
    CommonViews::ShaderParameter(settings->tolerance, settings->toleranceOscillator);
    CommonViews::ShaderColor(settings->color);
  }
};

#endif /* ColorKeyMaskMakerShader_hpp */
