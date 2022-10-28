//
//  PixelShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PixelShader_hpp
#define PixelShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct PixelShader: Shader {
  PixelSettings *settings;
  PixelShader(PixelSettings *settings) : settings(settings), Shader(settings) {};
  
  ofShader shader;
  void setup() override {
    shader.load("shadersGL2/new/pixel");
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("size", settings->size.value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }
  
  bool enabled() override {
    return settings->enabled.boolValue;
  }
  
  std::string name() override {
    return "Pixel";
  }
  
  ShaderType type() override {
    return ShaderTypePixelate;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Pixelation");
    
    ImGui::Text("Mix");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##pixelation_enabled", &settings->enabled.boolValue);
    
    CommonViews::SliderWithOscillator("Size", "##pixelation_size", &settings->size, &settings->sizeOscillator);
    CommonViews::ModulationSelector(&settings->size);
    CommonViews::MidiSelector(&settings->size);
  }
};

#endif /* PixelShader_hpp */
