//
//  OscillatorPanelView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/22/24.
//

#include "CommonViews.hpp"
#include "OscillatorPanelView.hpp"
#include "OscillationService.hpp"
#include "OscillatorView.hpp"
#include "imgui.h"

void OscillatorPanelView::setup() {
  // Setup code if needed
}

void OscillatorPanelView::update() {
  // Update code if needed
}

void OscillatorPanelView::draw() {
  auto oscillators = OscillationService::getService()->oscillators;
  
  ImGui::BeginChild("##oscillatorPanel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
  
  for (const auto& [key, oscillator] : oscillators) {
    if (oscillator->enabled->boolValue) {
      CommonViews::H3Title(formatString("%s - %s", oscillator->value->name.c_str(), oscillator->value->ownerName.c_str()));
      OscillatorView::draw(oscillator, oscillator->value);
    }
  }
  
  ImGui::EndChild();
}
