//
//  FishEyeShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FishEyeShader_hpp
#define FishEyeShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FishEyeSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<Oscillator> amountOscillator;
  
  FishEyeSettings(std::string shaderId, json j) :
  amount(std::make_shared<Parameter>("amount", 0.5, 0.0, 1000.0)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j) {
    
  };
};

class FishEyeShader: public Shader {
public:

  FishEyeSettings *settings;
  FishEyeShader(FishEyeSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/FishEye");
#endif
#ifdef RELEASE
shader.load("shaders/FishEye");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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
    return ShaderTypeFishEye;
  }

  void drawSettings() override {
    
    CommonViews::H3Title("FishEye");
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* FishEyeShader_hpp */