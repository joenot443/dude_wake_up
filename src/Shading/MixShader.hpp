//
//  MixShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/13/22.
//

#ifndef MixShader_h
#define MixShader_h

#include "ofMain.h"

#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "VideoSourceService.hpp"
#include "Oscillator.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct MixSettings : public ShaderSettings {
  std::shared_ptr<Parameter> mix;
  std::string mixSourceId = "";
  std::shared_ptr<Oscillator> mixOscillator;

  MixSettings(std::string shaderId, json j) :
  mixSourceId(mixSourceId),
  mix(std::make_shared<Parameter>("mix", shaderId, 0.5, 0.0, 1.0)),
  mixOscillator(std::make_shared<WaveformOscillator>(mix)),
  ShaderSettings(shaderId) {
    parameters = {mix};
    oscillators = {mixOscillator};
    load(j);
  };
};

struct MixShader : public Shader {
  ofShader shader;
  MixSettings *settings;
  std::shared_ptr<VideoSource> mixSource;

  MixShader(MixSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("../../shaders/Mix");
    mixSource = VideoSourceService::getService()->defaultVideoSource();
    settings->mixSourceId = mixSource->id;
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();

    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("tex2_mix", settings->mix->value);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniformTexture("tex2", *mixSource->frameTexture, 3);
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  ShaderType type() override {
    return ShaderType::ShaderTypeMix;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Mix");
    CommonViews::ShaderParameter(settings->mix, settings->mixOscillator);
    
    ImGui::Text("Feedback Source");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    // Get the list of available sources.
    std::vector<std::shared_ptr<VideoSource>> sources = VideoSourceService::getService()->videoSources();
    
    // Draw the selector.
    if (ImGui::BeginCombo("##feedbackSource", mixSource->sourceName.c_str())) {
      for (int i = 0; i < sources.size(); i++) {
        // Get the id of the source.
        auto source = sources[i];
        bool isSelected = (settings->mixSourceId == source->id);
        if (ImGui::Selectable(source->sourceName.c_str(), isSelected)) {
          settings->mixSourceId = source->id;
          mixSource = source;
        }
        
        if (isSelected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }
};

#endif /* MixShader_h */
