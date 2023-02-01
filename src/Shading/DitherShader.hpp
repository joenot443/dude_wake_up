//
//  DitherShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DitherShader_hpp
#define DitherShader_hpp

#include "CommonViews.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct DitherSettings : public ShaderSettings {
  std::shared_ptr<Parameter> shape;

  std::shared_ptr<Oscillator> shapeOscillator;

  DitherSettings(std::string shaderId, json j)
      : shape(std::make_shared<Parameter>("shape", shaderId, 0.0, 0.0, 1.0)),
        shapeOscillator(std::make_shared<WaveformOscillator>(shape)),
        ShaderSettings(shaderId) {
    parameters = {shape};
    oscillators = {shapeOscillator};
  };
};

struct DitherShader : Shader {
  DitherSettings *settings;
  DitherShader(DitherSettings *settings)
      : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override { shader.load("shaders/Dither"); }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("shape", settings->shape->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

  ShaderType type() override { return ShaderTypeDither; }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->shape, settings->shapeOscillator);
  }
};

#endif /* DitherShader_hpp */
