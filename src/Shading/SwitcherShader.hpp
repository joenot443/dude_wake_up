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

struct SwitcherSettings: public ShaderSettings {
  std::shared_ptr<Parameter> selectedIndex;
  std::shared_ptr<Parameter> delay;

  SwitcherSettings(std::string shaderId, json j) :
  selectedIndex(std::make_shared<Parameter>("Output", 0.0, 0.0, 100.0)),
  delay(std::make_shared<Parameter>("Delay", 0.0, 0.0, 10.0)),
  ShaderSettings(shaderId, j) {
    parameters = { selectedIndex, delay };
    oscillators = {};
    load(j);
    registerParameters();
  };
};

struct SwitcherShader: Shader {
  SwitcherSettings *settings;
  std::vector<std::shared_ptr<Connectable>> activeInputs = {};
  SwitcherShader(SwitcherSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  double lastSwitchTime;
  bool isPlaying;
  
  void setup() override {
    lastSwitchTime = ofGetElapsedTimef();
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    activeInputs.clear();
    
    // Accumulate the connected inputs
    for (auto [slot, conn] : inputs) {
      activeInputs.push_back(conn->start);
    }
    // Once settings->delay->value has passed, increment selectedIndex.
    if (isPlaying &&
        ofGetElapsedTimef() - lastSwitchTime > settings->delay->value) {
      settings->selectedIndex->intValue = (settings->selectedIndex->intValue + 1) % activeInputs.size();
      lastSwitchTime = ofGetElapsedTimef();
    }
    
    if (activeInputs.size() == 0) {
      frame->draw(0, 0);
      canvas->end();
      return;
    }
    
    std::shared_ptr<Connectable> selectedConnectable = activeInputs[settings->selectedIndex->intValue];
    selectedConnectable->frame()->draw(0, 0);
    canvas->end();
  }

  void clear() override {
    
  }

  int inputCount() override {
    return std::max(2, std::min(10, static_cast<int>(activeInputs.size() + 1)));
  }

  ShaderType type() override {
    return ShaderTypeSwitcher;
  }

  void drawSettings() override {
    CommonViews::H3Title("Switcher");
    CommonViews::ShaderParameter(settings->delay, nullptr);
    ImGui::SameLine();
    auto playIcon = isPlaying ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
    if (CommonViews::IconButton(playIcon, shaderId)) {
      isPlaying = !isPlaying;
    }
    drawSelector();
  }
  
  void drawSelector() {
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

#endif /* SwitcherShader_hpp */
