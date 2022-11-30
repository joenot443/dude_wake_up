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
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<Oscillator> speedOscillator;

  FractalSettings(std::string shaderId, json j) :
  speed(std::make_shared<Parameter>("speed", shaderId, 0.0,  0.0, 1.0)),
  speedOscillator(std::make_shared<Oscillator>(speed)),
  ShaderSettings(shaderId) {
    parameters = {speed};
    oscillators = {speedOscillator};
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
    shader.setUniform1f("speed", settings->speed->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shadersGL2/new/fractal");
  };

  void drawSettings() override {
    CommonViews::H3Title("Fractal");
    ImGui::Text("Fractal Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
//    ImGui::Checkbox("Enabled##fractal_enabled", &settings->enabled.boolValue);
  };
};

#endif /* FractalShader_h */
