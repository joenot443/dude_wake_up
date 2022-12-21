//
//  TransformShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef TransformShader_hpp
#define TransformShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

// Transform

struct TransformSettings: public ShaderSettings {
  std::shared_ptr<Parameter> xTranslate;
  std::shared_ptr<Parameter> yTranslate;
  std::shared_ptr<Parameter> xScale;
  std::shared_ptr<Parameter> yScale;
  std::shared_ptr<Parameter> rotate;
  
  std::shared_ptr<Oscillator> xTranslateOscillator;
  std::shared_ptr<Oscillator> yTranslateOscillator;
  std::shared_ptr<Oscillator> xScaleOscillator;
  std::shared_ptr<Oscillator> yScaleOscillator;
  std::shared_ptr<Oscillator> rotateOscillator;

  TransformSettings(std::string shaderId, json j) :
  xTranslate(std::make_shared<Parameter>("xTranslate", shaderId, 0.0,  -1.0, 1.0)),
  yTranslate(std::make_shared<Parameter>("yTranslate", shaderId, 0.0,  -1.0, 1.0)),
  xScale(std::make_shared<Parameter>("xScale", shaderId, 1.0,  0.0, 2.0)),
  yScale(std::make_shared<Parameter>("yScale", shaderId, 1.0,  0.0, 2.0)),
  rotate(std::make_shared<Parameter>("rotate", shaderId, 0.0,  0.0, 360.0)),
  xTranslateOscillator(std::make_shared<Oscillator>(xTranslate)),
  yTranslateOscillator(std::make_shared<Oscillator>(yTranslate)),
  xScaleOscillator(std::make_shared<Oscillator>(xScale)),
  yScaleOscillator(std::make_shared<Oscillator>(yScale)),
  rotateOscillator(std::make_shared<Oscillator>(rotate)),
  ShaderSettings(shaderId)
  {
    parameters = {xTranslate, yTranslate, xScale, yScale, rotate};
    oscillators = {xTranslateOscillator, yTranslateOscillator, xScaleOscillator, yScaleOscillator, rotateOscillator};
    load(j);
  }
};

struct TransformShader: public Shader {
public:
  ofShader shader;
  TransformSettings *settings;
  
  TransformShader(TransformSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {}
  
  ShaderType type() override {
    return ShaderTypeTransform;
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    auto texture = frame->getTexture();
    float width = frame->getWidth();
    float height = frame->getHeight();
    ofClear(0,0,0,255);
    
    ofSetRectMode(OF_RECTMODE_CENTER);
    ofPushMatrix();
    
    float x = (settings->xTranslate->value + 0.5) * width;
    float y = (settings->yTranslate->value + 0.5) * height;
    ofTranslate(x, y);
    ofRotateDeg(settings->rotate->value);
    ofScale(settings->xScale->value, settings->yScale->value);
    texture.draw(0, 0);
    ofPopMatrix();

    shader.end();
    canvas->end();
  }

  
  void drawSettings() override {
    CommonViews::H3Title("Transform");

    // Translate X
    ImGui::Text("Translate X");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Translate X", "##xoffset", settings->xTranslate);
    CommonViews::MidiSelector(settings->xTranslate);

    // Translate Y
    ImGui::Text("Translate Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Translate Y", "##yoffset", settings->yTranslate);
    CommonViews::MidiSelector(settings->yTranslate);

    // Scale X
    ImGui::Text("Scale X");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Scale X", "##xscale", settings->xScale);
    CommonViews::MidiSelector(settings->xScale);

    // Scale Y
    ImGui::Text("Scale Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Scale Y", "##yscale", settings->yScale);
    CommonViews::MidiSelector(settings->yScale);

    // Rotate
    ImGui::Text("Rotate");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::Slider("Rotate", "##rotate", settings->rotate);
    CommonViews::MidiSelector(settings->rotate);
  }

  void clear() override {
    
  }

};

#endif
