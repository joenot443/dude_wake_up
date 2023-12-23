//
//  PlasmaTwoShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PlasmaTwoShader_hpp
#define PlasmaTwoShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PlasmaTwoSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> color;
  std::shared_ptr<WaveformOscillator> colorOscillator;
  
  PlasmaTwoSettings(std::string shaderId, json j) :
  color(std::make_shared<Parameter>("color", 1.0  , -1.0, 2.0)),
  colorOscillator(std::make_shared<WaveformOscillator>(color)),
  ShaderSettings(shaderId, j)
  {
    parameters = { color };
    oscillators = { colorOscillator };
    load(j);
    registerParameters();
  };
};

class PlasmaTwoShader : public Shader
{
public:
  PlasmaTwoSettings *settings;
  PlasmaTwoShader(PlasmaTwoSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override
  {
    shader.load("shaders/PlasmaTwo");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->color->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

  ShaderType type() override
  {
    return ShaderTypePlasmaTwo;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("PlasmaTwo");
    CommonViews::ShaderParameter(settings->color, settings->colorOscillator);
  }
};

#endif /* PlasmaTwoShader_hpp */
