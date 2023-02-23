//
//  ColorPassShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorPassShader_hpp
#define ColorPassShader_hpp

#include "CommonViews.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ColorPassSettings : public ShaderSettings {
  std::shared_ptr<Parameter> lowHue;
  std::shared_ptr<Parameter> highHue;

  std::shared_ptr<Oscillator> lowHueOsc;
  std::shared_ptr<Oscillator> highHueOsc;

  ColorPassSettings(std::string shaderId, json j)
      : lowHue(std::make_shared<Parameter>("low_hue", shaderId, 0.0, 0.0, 1.0)),
        highHue(
            std::make_shared<Parameter>("high_hue", shaderId, 0.5, 0.0, 1.0)),
        lowHueOsc(std::make_shared<WaveformOscillator>(lowHue)),
        highHueOsc(std::make_shared<WaveformOscillator>(highHue)),
        ShaderSettings(shaderId){

        };
};

struct ColorPassShader : Shader {
  ColorPassSettings *settings;
  ColorPassShader(ColorPassSettings *settings)
      : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override { shader.load("shaders/ColorPass"); }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    shader.setUniform1f("hue1", settings->lowHue->value);
    shader.setUniform1f("hue2", settings->highHue->value);
    shader.setUniform1i("invert", 0);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {}

  ShaderType type() override { return ShaderTypeColorPass; }

  void drawSettings() override {
    CommonViews::H3Title("ColorPass");
    CommonViews::ShaderParameter(settings->lowHue, settings->lowHueOsc);
    CommonViews::ShaderParameter(settings->highHue, settings->highHueOsc);
  }
};

#endif /* ColorPassShader_hpp */
