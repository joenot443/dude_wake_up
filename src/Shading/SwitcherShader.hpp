//
//  SwitcherShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef SwitcherShader_hpp
#define SwitcherShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SwitcherSettings : public ShaderSettings {
  std::shared_ptr<Parameter> selectedIndex;
  std::shared_ptr<Parameter> delay;
  std::shared_ptr<Parameter> transitionDuration;
  std::shared_ptr<Parameter> blendMode;
  
  SwitcherSettings(std::string shaderId, json j) :
  selectedIndex(std::make_shared<Parameter>("Output", 0.0, 0.0, 100.0)),
  delay(std::make_shared<Parameter>("Delay", 0.0, 0.0, 10.0)),
  transitionDuration(std::make_shared<Parameter>("Transition Duration", 1.0, 0.0, 5.0)),
  blendMode(std::make_shared<Parameter>("Blend Mode", 0.0, 0.0, 11.0)),
  ShaderSettings(shaderId, j, "Switcher") {
    parameters = { selectedIndex, delay, transitionDuration };
    oscillators = {};
    load(j);
    registerParameters();
  };
};

struct SwitcherShader : Shader {
  SwitcherSettings* settings;
  std::vector<std::shared_ptr<Connectable>> activeInputs = {};

  double lastSwitchTime;
  bool isPlaying;
  float transitionProgress;
  std::shared_ptr<ofFbo> previousFbo;
  
  SwitcherShader(SwitcherSettings* settings) : settings(settings), Shader(settings) {};
  
  void clear() override {
    
  }
  
  int inputCount() override {
    return std::max(2, std::min(10, static_cast<int>(activeInputs.size() + 1)));
  }
  
  ShaderType type() override {
    return ShaderTypeSwitcher;
  }
  
  void setup() override {
    shader.load("shaders/Mix");
    lastSwitchTime = ofGetElapsedTimef();
    transitionProgress = 0.0;
    previousFbo = std::make_shared<ofFbo>();
    previousFbo->allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
  }
  
  void drawBlend(std::shared_ptr<ofFbo> fbo1, std::shared_ptr<ofFbo> fbo2, float mix) {
    shader.begin();
    shader.setUniformTexture("tex", fbo1->getTexture(), 0);
    shader.setUniformTexture("tex2", fbo2->getTexture(), 1);
    shader.setUniform1f("tex2_mix", mix);
    fbo1->draw(0, 0);
    shader.end();
  }
  
  void drawSingle(std::shared_ptr<ofFbo> fbo1) {
    fbo1->draw(0, 0);
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    activeInputs.clear();
    
    // Accumulate the connected inputs
    for (auto& [slot, conn] : inputs) {
      if (conn)
        activeInputs.push_back(conn->start);
    }
    
    // If no active inputs, draw the existing frame
    if (activeInputs.empty()) {
      frame->draw(0, 0);
      canvas->end();
      return;
    }
    
    double currentTime = ofGetElapsedTimef();
    bool shouldSwitch = (currentTime - lastSwitchTime > settings->delay->value + settings->transitionDuration->value) && isPlaying;
    
    // Check if it's time to switch inputs
    if (shouldSwitch) {
      previousFbo = activeInputs[settings->selectedIndex->intValue]->frame();
      int nextIndex = (settings->selectedIndex->intValue + 1) % activeInputs.size();
      settings->selectedIndex->intValue = nextIndex;
      lastSwitchTime = currentTime;
      transitionProgress = 0.0;  // Reset transition progress on switch
    }
    
    
    // Check if we're using a transition
    if (settings->transitionDuration->value <= 0.1f) {
      std::shared_ptr<ofFbo> currentFbo = activeInputs[settings->selectedIndex->intValue]->frame();
      drawSingle(currentFbo);
      canvas->end();
      return;
    }
    
    transitionProgress += ofGetLastFrameTime() / settings->transitionDuration->value;
    std::shared_ptr<ofFbo> currentFbo = activeInputs[settings->selectedIndex->intValue]->frame();
    
    // Handle transition logic
    if (transitionProgress < 1.0) {
      drawBlend(previousFbo, currentFbo, transitionProgress);
    }
    
    // Transition is complete
    if (transitionProgress >= 1.0) {
      drawSingle(currentFbo);
    }
    
    canvas->end();
  }
  
  void drawSettings() override {
    
    CommonViews::ShaderParameter(settings->delay, nullptr);
    ImGui::SameLine();
    auto playIcon = isPlaying ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
    if (CommonViews::IconButton(playIcon, shaderId)) {
      isPlaying = !isPlaying;
    }
    CommonViews::ShaderParameter(settings->transitionDuration, nullptr);
    //    CommonViews::BlendModeSelector(settings->blendMode);
    drawSelector();
  }
  
  void drawSelector() {
    if (activeInputs.empty()) return;
    
    if (ImGui::BeginTable("split1", (int) activeInputs.size(), ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame))
    {
      for (int i = 0; i < activeInputs.size(); i++)
      {
        ImGui::TableNextColumn();
        if (ImGui::Selectable(activeInputs[i]->name().c_str(), settings->selectedIndex->intValue == i)) {
          settings->selectedIndex->intValue = i;
        }
      }
      ImGui::EndTable();
    }
  }
};

#endif
