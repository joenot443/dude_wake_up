//
//  HalfToneShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HalfToneShader_hpp
#define HalfToneShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct HalfToneSettings: public ShaderSettings {
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> radius;
  
  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> radiusOscillator;
  
  HalfToneSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", shaderId, 0., 0., 10.)),
  radius(std::make_shared<Parameter>("radius", shaderId, 0., 0., 10.)),
  
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  ShaderSettings(shaderId) {
    
  };
};

struct HalfToneShader: Shader {
  HalfToneSettings *settings;
  HalfToneShader(HalfToneSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/HalfTone");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("radius", settings->radius->value);

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeHalfTone;
  }

  void drawSettings() override {
    CommonViews::H3Title("HalfTone");
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
  }
};

#endif /* HalfToneShader_hpp */
