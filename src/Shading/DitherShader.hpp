//
//  DitherShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef DitherShader_hpp
#define DitherShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct DitherSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> shape;

  std::shared_ptr<Oscillator> shapeOscillator;

  DitherSettings(std::string shaderId, json j, std::string name)
      : shape(std::make_shared<Parameter>("shape", 0.0, 0.0, 1.0)),
        shapeOscillator(std::make_shared<WaveformOscillator>(shape)),
        ShaderSettings(shaderId, j, name) {
    parameters = {shape};
    oscillators = {shapeOscillator};
  };
};

class DitherShader : public Shader {
public:

  DitherSettings *settings;
  DitherShader(DitherSettings *settings)
      : settings(settings), Shader(settings){};

  void setup() override {
#ifdef TESTING
shader.load("shaders/Dither");
#endif
#ifdef RELEASE
shader.load("shaders/Dither");
#endif
    
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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

    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypeDither; }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->shape, settings->shapeOscillator);
  }
};

#endif /* DitherShader_hpp */
