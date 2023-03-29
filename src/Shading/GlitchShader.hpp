//
//  GlitchShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef GlitchShader_hpp
#define GlitchShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct GlitchSettings: public ShaderSettings {
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Oscillator> amountOscillator;
  
  GlitchSettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("amount", shaderId, 0.5, 0.0, 1.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId) {
    
  };
};

struct GlitchShader: Shader {
  GlitchSettings *settings;
  GlitchShader(GlitchSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Glitch");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeGlitch;
  }

  void drawSettings() override {
    CommonViews::H3Title("Glitch");
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* GlitchShader_hpp */
