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
    
    float x = (settings->xTranslate.value + 0.5) * width;
    float y = (settings->yTranslate.value + 0.5) * height;
    ofTranslate(x, y);
    ofRotateDeg(settings->rotate.value);
    ofScale(settings->xScale.value, settings->yScale.value);
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
    CommonViews::SliderWithOscillator("Translate X", "##xoffset", &settings->xTranslate, &settings->xTranslateOscillator);
    CommonViews::MidiSelector(&settings->xTranslate);

    // Translate Y
    ImGui::Text("Translate Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::SliderWithOscillator("Translate Y", "##yoffset", &settings->yTranslate, &settings->yTranslateOscillator);
    CommonViews::MidiSelector(&settings->yTranslate);

    // Scale X
    ImGui::Text("Scale X");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::SliderWithOscillator("Scale X", "##xscale", &settings->xScale, &settings->xScaleOscillator);
    CommonViews::MidiSelector(&settings->xScale);

    // Scale Y
    ImGui::Text("Scale Y");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::SliderWithOscillator("Scale Y", "##yscale", &settings->yScale, &settings->yScaleOscillator);
    CommonViews::MidiSelector(&settings->yScale);

    // Rotate
    ImGui::Text("Rotate");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    CommonViews::SliderWithOscillator("Rotate", "##rotate", &settings->rotate, &settings->rotateOscillator);
    CommonViews::MidiSelector(&settings->rotate);
  }

  void clear() override {
    
  }

};

#endif
