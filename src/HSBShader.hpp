//
//  HSBShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef HSBShader_hpp
#define HSBShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct HSBShader: Shader {
  
  ofShader shader;
public:
  HSBShader(HSBSettings *settings) : Shader(settings), settings(settings) {};
  
  HSBSettings *settings;
  
  void setup() override {
    shader.load("shadersGL2/new/hsb");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform3f("hsbScalar", settings->hue.value, settings->saturation.value, settings->brightness.value);
    shader.setUniform3f("hsbFloor", 0.3, 0.3, 0.3);
    shader.setUniform3f("hsbCeil", 1.0, 1.0, 1.0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderTypeHSB;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Basic (HSB)");
    
    // Hue
    CommonViews::SliderWithOscillator("Hue", "##hue", &settings->hue, &settings->hueOscillator);
    CommonViews::ModulationSelector(&settings->hue);
    CommonViews::MidiSelector(&settings->hue);
    
    // Saturation
    CommonViews::SliderWithOscillator("Saturation", "##saturation", &settings->saturation, &settings->saturationOscillator);
    CommonViews::ModulationSelector(&settings->saturation);
    CommonViews::MidiSelector(&settings->saturation);
    
    // Brightness
    CommonViews::SliderWithOscillator("Brightness", "##brightness", &settings->brightness, &settings->brightnessOscillator);
    CommonViews::ModulationSelector(&settings->brightness);
    CommonViews::MidiSelector(&settings->brightness);

  }
};

#endif /* HSBShader_hpp */
