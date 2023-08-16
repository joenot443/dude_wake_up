//
//  FractalShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef FractalShader_h
#define FractalShader_h

#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
 
#include "ShaderSettings.hpp"
#include "ofxImGui.h"

struct FractalSettings: public ShaderSettings {
  std::shared_ptr<Parameter> zoom;

  std::shared_ptr<Oscillator> zoomOscillator;

  FractalSettings(std::string shaderId, json j) :
  zoom(std::make_shared<Parameter>("zoom", 1.0, 1.0, 5.0)),
  zoomOscillator(std::make_shared<WaveformOscillator>(zoom)),
  ShaderSettings(shaderId, j) {
    parameters = {zoom};
    oscillators = {zoomOscillator};
    load(j);
  registerParameters();
  };
};


class FractalShader: public Shader {
  private:
  ofShader shader;

public:
  FractalShader(FractalSettings *settings) :
  settings(settings),
  Shader(settings) {};

  FractalSettings *settings;

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1f("zoom", settings->zoom->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    #ifdef TESTING
shader.load("shaders/Fractal");
#endif
#ifdef RELEASE
shader.load("shaders/Fractal");
#endif
  };

  void drawSettings() override {
    ShaderConfigSelectionView::draw(this);
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
  };
};

#endif /* FractalShader_h */
