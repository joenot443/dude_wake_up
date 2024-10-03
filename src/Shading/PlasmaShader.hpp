//
//  PlasmaShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/21/22.
//

#ifndef PlasmaShader_h
#define PlasmaShader_h

#include <stdio.h>
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "CommonViews.hpp"
#include "ShaderSettings.hpp"

#include "CommonViews.hpp"

// Plasma

struct PlasmaSettings: public ShaderSettings {
	public:
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> color;

  std::shared_ptr<Oscillator> speedOscillator;
  std::shared_ptr<Oscillator> colorOscillator;

  PlasmaSettings(std::string shaderId, json j, std::string name) :
  speed(std::make_shared<Parameter>("speed", 1.0,  0.0, 5.0)),
  color(std::make_shared<Parameter>("color", 1.0,  0.0, 1.0)),
  speedOscillator(std::make_shared<WaveformOscillator>(speed)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  ShaderSettings(shaderId, j, name) {
    parameters = {speed, color};
    oscillators = {speedOscillator, colorOscillator};
    load(j);
    registerParameters();
   };
};

class PlasmaShader : public Shader {
private:


public:
 PlasmaShader(PlasmaSettings* settings) :
  settings(settings),
  Shader(settings) {};
  
  PlasmaSettings *settings;
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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
    #ifdef TESTING
shader.load("shaders/plasma");
#endif
#ifdef RELEASE
shader.load("shaders/plasma");
#endif
  };
  
  int inputCount() override {
    return 0;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
  };
};


#endif /* PlasmaShader_h */
