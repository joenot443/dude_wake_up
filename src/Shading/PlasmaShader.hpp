//
//  PlasmaShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef PlasmaShader_h
#define PlasmaShader_h

#include <stdio.h>
#include "Shader.hpp"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"

#include "CommonViews.hpp"

// Plasma

struct PlasmaSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enabled;
  std::shared_ptr<Parameter> plasmaX;
  std::shared_ptr<Parameter> plasmaY;

  std::shared_ptr<Oscillator> plasmaXOscillator;
  std::shared_ptr<Oscillator> plasmaYOscillator;

  PlasmaSettings(std::string shaderId, json j) :
  enabled(std::make_shared<Parameter>("enabled", shaderId, 0.0,  1.0, 0.0)),
  plasmaX(std::make_shared<Parameter>("plasmaX", shaderId, 0.0,  0.0, 1.0)),
  plasmaY(std::make_shared<Parameter>("plasmaY", shaderId, 0.0,  0.0, 1.0)),
  plasmaXOscillator(std::make_shared<WaveformOscillator>(plasmaX)),
  plasmaYOscillator(std::make_shared<WaveformOscillator>(plasmaY)),
  ShaderSettings(shaderId) {
    parameters = {enabled, plasmaX, plasmaY};
    oscillators = {plasmaXOscillator, plasmaYOscillator};
    load(j);
   };
};

class PlasmaShader : public Shader {
private:
  ofShader shader;

public:
  PlasmaShader(PlasmaSettings* settings) :
  settings(settings),
  Shader(settings) {};
  
  PlasmaSettings *settings;

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform2f("plasma", frame->getWidth() * settings->plasmaX->value, frame->getHeight() * settings->plasmaY->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shaders/plasma");
  };

  void drawSettings() override {
    CommonViews::Slider("Plasma", "##plasmaX", settings->plasmaX);
    
    CommonViews::Slider("Plasma", "##plasmaY", settings->plasmaY);
  };
};


#endif /* PlasmaShader_h */
