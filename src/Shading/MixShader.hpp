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
  std::shared_ptr<Parameter> mix;
  std::string mixSourceId = "";
  std::shared_ptr<Oscillator> mixOscillator;

  MixSettings(std::string shaderId, json j) :
  mixSourceId(mixSourceId),
  mix(std::make_shared<Parameter>("mix", 0.5, 0.0, 1.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId, j) {
    parameters = {mix};
    oscillators = {mixOscillator};
    load(j);
  registerParameters();
  };
};

struct MixShader : public Shader {
  ofShader shader;
  MixSettings *settings;

  MixShader(MixSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    #ifdef TESTING
shader.load("shaders/Mix");
#endif
#ifdef RELEASE
shader.load("shaders/Mix");
#endif
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    ofEnableAlphaBlending();
    canvas->begin();
    shader.begin();
    
    if (auxConnected()) {
      ofTexture tex2 = auxTexture();
      shader.setUniformTexture("tex2", tex2, 8);
    } else {
      ofSetColor(0, 0, 0, 0);
      ofDrawRectangle(0, 0, canvas->getWidth(), canvas->getHeight());
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
    ShaderConfigSelectionView::draw(this);
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
  }
  
};

#endif /* MixShader_h */
