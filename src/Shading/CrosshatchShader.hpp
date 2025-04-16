//
//  CrosshatchShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef CrosshatchShader_hpp
#define CrosshatchShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct CrosshatchSettings: public ShaderSettings {
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Oscillator> mixOscillator;
  
	public:
  CrosshatchSettings(std::string shaderId, json j, std::string name) :
  mix(std::make_shared<Parameter>("Amount", 1.0, 0.0, 10.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId, j, name) {
    parameters = { mix };
    oscillators = { mixOscillator };
    audioReactiveParameter = mix;
    registerParameters();
  };
};

struct CrosshatchShader: Shader {
  CrosshatchSettings *settings;
  CrosshatchShader(CrosshatchSettings *settings) : settings(settings), Shader(settings) {
  };

  void setup() override {
shader.load("shaders/Crosshatch");
shader.load("shaders/Crosshatch");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    ofClear(0,0,0,255);
    ofClear(0,0,0,0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("amount", settings->mix->value);
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
    return ShaderTypeCrosshatch;
  }

  void drawSettings() override {
    
    
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
  }
};

#endif
