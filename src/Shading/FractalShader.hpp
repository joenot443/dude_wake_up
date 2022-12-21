//
//  FractalShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef FractalShader_h
#define FractalShader_h

#include "Shader.hpp"
#include "CommonViews.hpp"
#include "VideoSettings.hpp" 
#include "ShaderSettings.hpp"
#include "ofxImGui.h"

struct FractalSettings: public ShaderSettings {
  std::shared_ptr<Parameter> zoom;

  std::shared_ptr<Oscillator> zoomOscillator;

  FractalSettings(std::string shaderId, json j) :
  zoom(std::make_shared<Parameter>("zoom", shaderId, 1.0, 1.0, 5.0)),
  zoomOscillator(std::make_shared<Oscillator>(zoom)),
  ShaderSettings(shaderId) {
    parameters = {zoom};
    oscillators = {zoomOscillator};
    load(j);
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
    shader.load("shaders/fractal");
  };

  void drawSettings() override {
    CommonViews::H3Title("Fractal");
    CommonViews::ShaderParameter(settings->zoom, settings->zoomOscillator);
  };
};

#endif /* FractalShader_h */
