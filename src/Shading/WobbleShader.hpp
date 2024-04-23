//
//  WobbleShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WobbleShader_hpp
#define WobbleShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "WaveformOscillator.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <string>
#include <stdio.h>

struct WobbleSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> amount;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> amountOscillator;

  WobbleSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("speed", 1.0, 0.0, 2.0)),
  amount(std::make_shared<Parameter>("amount", 1.0, 0.0, 2.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, name) {
    
  };
};

class WobbleShader: public Shader {
public:

  WobbleSettings *settings;
  WobbleShader(WobbleSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Wobble");
#endif
#ifdef RELEASE
shader.load("shaders/Wobble");
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

  ShaderType type() override {
    return ShaderTypeWobble;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif /* WobbleShader_hpp */
