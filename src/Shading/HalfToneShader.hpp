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
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct HalfToneSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> radius;
  
  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> radiusOscillator;
  
  HalfToneSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("speed", 0., 0., 10.)),
  radius(std::make_shared<Parameter>("radius", 0., 0., 10.)),
  
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
  ShaderSettings(shaderId, j, name) {
    
  };
};

class HalfToneShader: public Shader {
public:

  HalfToneSettings *settings;
  HalfToneShader(HalfToneSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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

    int inputCount() override {
    return 1;
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
