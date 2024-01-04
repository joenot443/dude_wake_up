//
//  BlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlurShader_hpp
#define BlurShader_hpp

#include "WaveformOscillator.hpp"
#include "ofMain.h"

#include "Math.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

struct BlurSettings : public ShaderSettings {
	public:
  std::string shaderId;
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Parameter> radius;
  std::shared_ptr<Oscillator> mixOscillator;
  std::shared_ptr<Oscillator> radiusOscillator;
  
  BlurSettings(std::string shaderId, json j)
      : mix(std::make_shared<Parameter>("blur_mix", 0.5, 0.0, 1.0)),
        radius(std::make_shared<Parameter>("blur_radius", 1.0, 0.0,
                                           5.0)),
        mixOscillator(std::make_shared<WaveformOscillator>(mix)),
        radiusOscillator(std::make_shared<WaveformOscillator>(radius)),
        shaderId(shaderId), ShaderSettings(shaderId, j) {
    parameters = {mix, radius};
    oscillators = {mixOscillator, radiusOscillator};
    load(j);
  registerParameters();
  }
};

class BlurShader : public Shader {
public:

  BlurSettings *settings;
  ofShader shader;

public:
  BlurShader(BlurSettings *settings) : Shader(settings), settings(settings) {}
  void setup() override {
#ifdef TESTING
shader.load("shaders/blur");
#endif
#ifdef RELEASE
shader.load("shaders/blur");
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
    shader.setUniform1f("size", settings->radius->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  ShaderType type() override { return ShaderTypeBlur; }

  void drawSettings() override {
    // Radius
    CommonViews::ShaderParameter(settings->radius, settings->radiusOscillator);
  }
};

#endif /* BlurShader_hpp */
