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
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> color;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> colorOscillator;

  PlasmaSettings(std::string shaderId, json j) :
  enabled(std::make_shared<Parameter>("enabled", shaderId, 0.0,  1.0, 0.0)),
  speed(std::make_shared<Parameter>("speed", shaderId, 1.0,  0.0, 5.0)),
  color(std::make_shared<Parameter>("color", shaderId, 1.0,  0.0, 20.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  ShaderSettings(shaderId) {
    parameters = {enabled, speed, color};
    oscillators = {speedOscillator, colorOscillator};
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
    frame->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    canvas->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform2f("sourceDimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("speed", settings->speed->value);
    shader.setUniform1f("color", settings->color->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shaders/plasma");
  };

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
  };
};


#endif /* PlasmaShader_h */
