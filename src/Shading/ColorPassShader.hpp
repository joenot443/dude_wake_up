//
//  ColorPassShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorPassShader_hpp
#define ColorPassShader_hpp

#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ColorPassSettings : public ShaderSettings {
	public:
  std::shared_ptr<Parameter> lowHue;
  std::shared_ptr<Parameter> highHue;

  std::shared_ptr<Oscillator> lowHueOsc;
  std::shared_ptr<Oscillator> highHueOsc;

  ColorPassSettings(std::string shaderId, json j, std::string name)
      : lowHue(std::make_shared<Parameter>("low_hue", 0.0, 0.0, 1.0)),
        highHue(
            std::make_shared<Parameter>("high_hue", 0.5, 0.0, 1.0)),
        lowHueOsc(std::make_shared<WaveformOscillator>(lowHue)),
        highHueOsc(std::make_shared<WaveformOscillator>(highHue)),
        ShaderSettings(shaderId, j, name){
          parameters = { lowHue, highHue };
          oscillators = { lowHueOsc, highHueOsc };
          load(j);
          registerParameters();
        };
};

class ColorPassShader : public Shader {
public:

  ColorPassSettings *settings;
  ColorPassShader(ColorPassSettings *settings)
      : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override {
#ifdef TESTING
shader.load("shaders/ColorPass");
#endif
#ifdef RELEASE
shader.load("shaders/ColorPass");
#endif
    
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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

    int inputCount() override {
    return 1;
  }
ShaderType type() override { return ShaderTypeColorPass; }

  void drawSettings() override {
    
    CommonViews::H3Title("ColorPass");
    CommonViews::ShaderParameter(settings->lowHue, settings->lowHueOsc);
    CommonViews::ShaderParameter(settings->highHue, settings->highHueOsc);
  }
};

#endif /* ColorPassShader_hpp */
