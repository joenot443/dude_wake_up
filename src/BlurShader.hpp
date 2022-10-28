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

struct BlurShader: Shader {
  BlurSettings *settings;
  ofShader shader;
public:
  BlurShader(BlurSettings *settings) : Shader(settings),
  settings(settings) {}
  void setup() override {
    shader.load("shadersGL2/new/blur");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1i("size", nearestOdd(settings->radius.value));
    shader.setUniform1f("blur_mix", settings->mix.value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  bool enabled() override {
    return true;
  }
  
  std::string name() override {
    return "Blur";
  }
  
  ShaderType type() override {
    return ShaderTypeBlur;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Blur");
    
    // Amount
    CommonViews::SliderWithOscillator("Mix", "##mix", &settings->mix, &settings->mixOscillator);
    CommonViews::ModulationSelector(&settings->mix);
    CommonViews::MidiSelector(&settings->mix);
    
    // Radius
    CommonViews::SliderWithOscillator("Radius", "##radius", &settings->radius, &settings->radiusOscillator);
    CommonViews::ModulationSelector(&settings->radius);
    CommonViews::MidiSelector(&settings->radius);
  }
};


#endif /* BlurShader_hpp */
