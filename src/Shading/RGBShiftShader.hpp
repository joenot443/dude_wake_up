//
//  RGBShiftShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef RGBShiftShader_hpp
#define RGBShiftShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "WaveformOscillator.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct RGBShiftSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> amount;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> amountOscillator;
  
  RGBShiftSettings(std::string shaderId, json j, std::string name) : 
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 5.0)),
  amount(std::make_shared<Parameter>("amount", 1.0, 0.0, 2.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, name)
  {
    parameters = {speed, amount};
    oscillators = {speedOscillator, amountOscillator};
    load(j);
    audioReactiveParameter = amount;
	  registerParameters();
  };
  
};

class RGBShiftShader: public Shader {
public:

  RGBShiftSettings *settings;
  RGBShiftShader(RGBShiftSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef DEBUG
shader.load("shaders/RGBShift");
#endif
#ifdef RELEASE
shader.load("shaders/RGBShift");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("amount", settings->amount->value);
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
    return ShaderTypeRGBShift;
  }

  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* RGBShiftShader_hpp */
