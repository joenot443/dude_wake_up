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

// Pixelate

struct PixelSettings: public ShaderSettings  {
  std::shared_ptr<Parameter> enabled;
  std::shared_ptr<Parameter> size;
  
  std::shared_ptr<Oscillator> sizeOscillator;
  
  PixelSettings(std::string shaderId, json j) :
  size(std::make_shared<Parameter>("pixel_size", shaderId, 24.0,  0.01, 64.0)),
  enabled(std::make_shared<Parameter>("enabled", shaderId, 0.0,  1.0, 0.0)),
  sizeOscillator(std::make_shared<Oscillator>(size)),
  ShaderSettings(shaderId)
  {
    parameters = {size, enabled};
    oscillators = {sizeOscillator};
    load(j);
  }
};


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
    shader.setUniform1f("size", settings->size->value);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  bool enabled() override {
    return settings->enabled->boolValue;
  }

  ShaderType type() override {
    return ShaderTypePixelate;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Pixelation");
    
    ImGui::Text("Mix");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##pixelation_enabled", &settings->enabled->boolValue);
    
    CommonViews::Slider("Size", "##pixelation_size", settings->size);
    CommonViews::ModulationSelector(settings->size);
    CommonViews::MidiSelector(settings->size);
  }
};

#endif /* PixelShader_hpp */
