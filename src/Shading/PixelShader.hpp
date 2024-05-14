//
//  PixelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelShader_hpp
#define PixelShader_hpp

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Pixelate

struct PixelSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> enabled;
  std::shared_ptr<Parameter> size;

  std::shared_ptr<Oscillator> sizeOscillator;

  PixelSettings(std::string shaderId, json j, std::string name)
      : size(std::make_shared<Parameter>("pixel_size", 24.1, 1.0,
                                         200.0)),
        enabled(
            std::make_shared<Parameter>("enabled", 0.0, 1.0, 0.0)),
        sizeOscillator(std::make_shared<WaveformOscillator>(size)),
        ShaderSettings(shaderId, j, name) {
    parameters = {size, enabled};
    oscillators = {sizeOscillator};
    load(j);
  registerParameters();
  }
};

class PixelShader : public Shader {
public:

  PixelSettings *settings;
  PixelShader(PixelSettings *settings) : settings(settings), Shader(settings){};

  ofShader shader;

  void setup() override {
#ifdef TESTING
shader.load("shaders/Pixel");
#endif
#ifdef RELEASE
shader.load("shaders/Pixel");
#endif
    
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("size", settings->size->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  bool enabled() override { return true; }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypePixelate; }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->size, settings->sizeOscillator);
  }
};

#endif /* PixelShader_hpp */
