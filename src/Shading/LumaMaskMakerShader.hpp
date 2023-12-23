//
//  LumaMaskMakerShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef LumaMaskMakerShader_hpp
#define LumaMaskMakerShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct LumaMaskMakerSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> upper;
  std::shared_ptr<Parameter> lower;
  
  std::shared_ptr<Oscillator> upperOscillator;
  std::shared_ptr<Oscillator> lowerOscillator;
  
  std::shared_ptr<Parameter> flip;
  
  LumaMaskMakerSettings(std::string shaderId, json j) :
  upper(std::make_shared<Parameter>("upper", 1.0, 0.0, 1.0)),
  lower(std::make_shared<Parameter>("lower", 0.5, 0.0, 1.0)),
  flip(std::make_shared<Parameter>("flip", 0.0, 0.0, 1.0)),
  upperOscillator(std::make_shared<WaveformOscillator>(upper)),
  lowerOscillator(std::make_shared<WaveformOscillator>(lower)),
  ShaderSettings(shaderId, 0) {
    parameters = {upper, lower, flip };
    oscillators = { upperOscillator, lowerOscillator };
    registerParameters();
  };
};

class LumaMaskMakerShader: public Shader {
public:

  LumaMaskMakerSettings *settings;
  LumaMaskMakerShader(LumaMaskMakerSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/LumaMaskMaker");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("lower", settings->lower->value);
    shader.setUniform1f("upper", settings->upper->value);
    shader.setUniform1i("flip", settings->flip->boolValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeLumaMaskMaker;
  }

  void drawSettings() override {
    CommonViews::H3Title("LumaMaskMaker");
    CommonViews::ShaderParameter(settings->upper, settings->upperOscillator);
    CommonViews::ShaderParameter(settings->lower, settings->lowerOscillator);
    CommonViews::ShaderCheckbox(settings->flip);
  }
};

#endif /* LumaMaskMakerShader_hpp */
