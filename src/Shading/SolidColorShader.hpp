//
//  SolidColorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SolidColorShader_hpp
#define SolidColorShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SolidColorSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Parameter> alpha;
  std::shared_ptr<Parameter> colorTransparentPixels;
  
  std::shared_ptr<WaveformOscillator> alphaOscillator;
  std::shared_ptr<WaveformOscillator> mixOscillator;

  SolidColorSettings(std::string shaderId, json j, std::string name) :
  color(std::make_shared<Parameter>("Color", 1.0, -1.0, 2.0)),
  mix(std::make_shared<Parameter>("Mix", 0.5, 0.0, 1.0)),
  alpha(std::make_shared<Parameter>("Alpha", 1.0, 0.0, 1.0)),
  colorTransparentPixels(std::make_shared<Parameter>("Color Transparent Pixels", 1.0, 0.0, 1.0)),
  alphaOscillator(std::make_shared<WaveformOscillator>(alpha)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId, j, name) {
    parameters = { color, colorTransparentPixels };
    load(j);
    registerParameters();
  };
};

class SolidColorShader: public Shader {
public:

  SolidColorSettings *settings;
  SolidColorShader(SolidColorSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/SolidColor");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
//    ofClear(0,0,0, 255);
//    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("colorMix", settings->mix->value);
    shader.setUniform1i("colorTransparentPixels", settings->colorTransparentPixels->intValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("color", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], settings->alpha->value);
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
    return ShaderTypeSolidColor;
  }

  void drawSettings() override {
    CommonViews::H3Title("SolidColor");

    CommonViews::ShaderColor(settings->color);
    CommonViews::ShaderParameter(settings->alpha, settings->alphaOscillator);
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
    CommonViews::ShaderCheckbox(settings->colorTransparentPixels);
  }
};

#endif /* SolidColorShader_hpp */
