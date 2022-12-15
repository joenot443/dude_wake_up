//
//  BlurShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlurShader_hpp
#define BlurShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Math.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct BlurSettings: public ShaderSettings  {
  std::string shaderId;
  std::shared_ptr<Parameter> mix;
  std::shared_ptr<Parameter> radius;
  
  std::shared_ptr<Oscillator> mixOscillator;
  std::shared_ptr<Oscillator> radiusOscillator;
  BlurSettings(std::string shaderId, json j) :
  mix(std::make_shared<Parameter>("blur_mix", shaderId, 0.0, 0.0, 1.0)),
  radius(std::make_shared<Parameter>("blur_radius", shaderId, 1.0, 0.0, 50.0)),
  mixOscillator(std::make_shared<Oscillator>(mix)),
  radiusOscillator(std::make_shared<Oscillator>(radius)),
  shaderId(shaderId),
  ShaderSettings(shaderId)  {
    parameters = {mix, radius};
    oscillators = {mixOscillator, radiusOscillator};
    load(j);
  }
};

struct BlurShader: Shader {
  BlurSettings *settings;
  ofShader shader;
public:
  BlurShader(BlurSettings *settings) : Shader(settings),
  settings(settings) {}
  void setup() override {
    shader.load("shaders/blur");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1i("size", nearestOdd(settings->radius->value));
    shader.setUniform1f("blur_mix", settings->mix->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderTypeBlur;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Blur");
    
    // Amount
    CommonViews::Slider("Mix", "##mix", settings->mix);
    CommonViews::ModulationSelector(settings->mix);
    CommonViews::MidiSelector(settings->mix);
    
    // Radius
    CommonViews::Slider("Radius", "##radius", settings->radius);
    CommonViews::ModulationSelector(settings->radius);
    CommonViews::MidiSelector(settings->radius);
  }
};


#endif /* BlurShader_hpp */
