//
//  MixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/13/22.
//

#ifndef MixShader_h
#define MixShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "Oscillator.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MixSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Oscillator> mixOscillator;

  MixSettings(std::string shaderId, json j, std::string name) :
  mix(std::make_shared<Parameter>("mix", 0.5, 0.0, 1.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId, j, name) {
    parameters = {mix};
    oscillators = {mixOscillator};
    load(j);
  registerParameters();
  };
};

class MixShader : public Shader {
public:

  ofShader shader;
  MixSettings *settings;
  int inputCount;

  MixShader(MixSettings *settings) : settings(settings), inputCount(2), Shader(settings) {};
  
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Mix");
#endif
#ifdef RELEASE
shader.load("shaders/Mix");
#endif
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    ofEnableAlphaBlending();
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    
    if (auxConnected()) {
      std::shared_ptr<ofFbo> tex2 = aux()->frame();
      shader.setUniformTexture("tex2", tex2->getTexture(), 8);
    } else {
      shader.end();
      frame->draw(0, 0);
      shader.end();
      canvas->end();
      return;
    }
    
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("tex2_mix", 1.0f - settings->mix->value);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderType::ShaderTypeMix;
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
  }
  
};

#endif /* MixShader_h */
