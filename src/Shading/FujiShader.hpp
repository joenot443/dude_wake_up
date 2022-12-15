//
//  FujiShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/22/22.
//

#ifndef FujiShader_h
#define FujiShader_h

#include "Shader.hpp"
#include "CommonViews.hpp"
#include "VideoSettings.hpp" 
#include "ShaderSettings.hpp"
#include "ofxImGui.h"

struct FujiSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enabled;
  
  std::shared_ptr<Parameter> speed;

  std::shared_ptr<Parameter> cloud1Y;
  std::shared_ptr<Parameter> cloud1X;

  std::shared_ptr<Parameter> cloud2Y;
  std::shared_ptr<Parameter> cloud2X;

  std::shared_ptr<Oscillator> cloud1XOscillator;
  std::shared_ptr<Oscillator> cloud1YOscillator;

  std::shared_ptr<Oscillator> cloud2XOscillator;
  std::shared_ptr<Oscillator> cloud2YOscillator;
  
  std::shared_ptr<Oscillator> speedOscillator;

  FujiSettings(std::string shaderId, json j) :
  enabled(std::make_shared<Parameter>("enabled", shaderId, 0.0,  1.0, 0.0)),
  cloud1Y(std::make_shared<Parameter>("cloud1Y", shaderId, 0.0,  -0.5, 0.5)),
  cloud1X(std::make_shared<Parameter>("cloud1X", shaderId, 0.0,  0.0, 1.0)),
  cloud2Y(std::make_shared<Parameter>("cloud2Y", shaderId, 0.0,  -0.5, 0.5)),
  cloud2X(std::make_shared<Parameter>("cloud2X", shaderId, 0.0,  0.0, 1.0)),
  speed(std::make_shared<Parameter>("speed", shaderId, 1.0, 0.0, 2.0)),
  cloud1XOscillator(std::make_shared<Oscillator>(cloud1X)),
  cloud1YOscillator(std::make_shared<Oscillator>(cloud1Y)),
  cloud2XOscillator(std::make_shared<Oscillator>(cloud2X)),
  cloud2YOscillator(std::make_shared<Oscillator>(cloud2Y)),
  speedOscillator(std::make_shared<Oscillator>(speed)),
  ShaderSettings(shaderId) {
    parameters = {enabled, cloud1Y, cloud1X, cloud2Y, cloud2X};
    oscillators = {cloud1XOscillator, cloud1YOscillator, cloud2XOscillator, cloud2YOscillator};
    load(j);
  };
};


class FujiShader: public Shader {
  private:
  ofShader shader;

public:
  FujiShader(FujiSettings *settings) :
  settings(settings),
  Shader(settings) {};

  FujiSettings *settings;

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("time", ofGetElapsedTimef() * settings->speed->value);
    shader.setUniform2f("cloud1", settings->cloud1X->value, settings->cloud1Y->value);
    shader.setUniform2f("cloud2", settings->cloud2X->value, settings->cloud2Y->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  };

  void setup() override {
    shader.load("shaders/fuji");
  };

  void drawSettings() override {
    CommonViews::H3Title("Fuji");

    ImGui::Text("Cloud 1 X");;
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("X", "##cloud1x", settings->cloud1X);
    CommonViews::MidiSelector(settings->cloud1X);
    CommonViews::OscillateButton("##x1", settings->cloud1XOscillator, settings->cloud1X);
  
    ImGui::Text("Cloud 1 Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Y", "##cloud1y", settings->cloud1Y);
    CommonViews::MidiSelector(settings->cloud1Y);
    CommonViews::OscillateButton("##y1", settings->cloud1YOscillator, settings->cloud1Y);


    ImGui::Text("Cloud 2 X");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("X", "##cloud2x", settings->cloud2X);
    CommonViews::MidiSelector(settings->cloud2X);
    CommonViews::OscillateButton("##x2", settings->cloud2XOscillator, settings->cloud2X);

    ImGui::Text("Cloud 2 Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Y", "##cloud2y", settings->cloud2Y);
    CommonViews::MidiSelector(settings->cloud2Y);
    CommonViews::OscillateButton("##y2", settings->cloud2YOscillator, settings->cloud2Y);

    CommonViews::ShaderParameter(settings->speed, settings->speedOscillator);
  }
};

#endif /* FujiShader_h */
