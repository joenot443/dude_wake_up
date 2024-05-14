//
//  KaleidoscopeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/27/22.
//

#ifndef KaleidoscopeShader_h
#define KaleidoscopeShader_h

#include <stdio.h>
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "Parameter.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"
#include "Oscillator.hpp"

struct KaleidoscopeSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> sides;
  std::shared_ptr<Parameter> shift;
  std::shared_ptr<Parameter> rotation;

  std::shared_ptr<Oscillator> sidesOscillator;
  std::shared_ptr<Oscillator> shiftOscillator;
  std::shared_ptr<Oscillator> rotationOscillator;

  KaleidoscopeSettings(std::string shaderId, json j, std::string name) :
  sides(std::make_shared<Parameter>("Sides", 4.0,  1.0, 10.0)),
  shift(std::make_shared<Parameter>("Frame Shift", 0.25,  0.0, 1.0)),
  rotation(std::make_shared<Parameter>("Rotation", 1.0,  1.0, TWO_PI)),
  sidesOscillator(std::make_shared<WaveformOscillator>(sides)),
  shiftOscillator(std::make_shared<WaveformOscillator>(shift)),
  rotationOscillator(std::make_shared<WaveformOscillator>(rotation)),
  ShaderSettings(shaderId, j, name) {
    parameters = {sides, shift, rotation};
    oscillators = {sidesOscillator, shiftOscillator, rotationOscillator};
    load(j);
  registerParameters();
  };
};

class KaleidoscopeShader : public Shader {
public:

  ofShader shader;
  KaleidoscopeSettings *settings;
  KaleidoscopeShader(KaleidoscopeSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef TESTING
shader.load("shaders/kaleidoscope");
#endif
#ifdef RELEASE
shader.load("shaders/kaleidoscope");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("sides", settings->sides->value);
    shader.setUniform1f("shift", settings->shift->value);
    shader.setUniform1f("rotation", settings->rotation->value);
    shader.setUniformTexture("tex", *frame, 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderType::ShaderTypeKaleidoscope;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->shift, settings->shiftOscillator);
    CommonViews::ShaderIntParameter(settings->sides);
    CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
  }
};


#endif /* KaleidoscopeShader_h */
