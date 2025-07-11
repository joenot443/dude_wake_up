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
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  amount(std::make_shared<Parameter>("Amount", 1.0, 0.0, 2.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  amountOscillator(std::make_shared<WaveformOscillator>(amount)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, amount};
    oscillators = {speedOscillator, amountOscillator};
    registerParameters();
  };
};

class WobbleShader: public Shader {
public:

  WobbleSettings *settings;
  WobbleShader(WobbleSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Wobble");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
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
    return ShaderTypeWobble;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
  }
};

#endif
