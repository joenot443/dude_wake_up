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
  auto oscillators = OscillationService::getService()->activeOscillators();
  
  ImGui::BeginChild("##oscillatorPanel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
  
  for (const auto& oscillator : oscillators) {
    if (oscillator->enabled->boolValue) {
      CommonViews::H3Title(formatString("%s - %s", oscillator->value->name.c_str(), oscillator->value->ownerName.c_str()));
      OscillatorView::draw(oscillator, oscillator->value);
    }
  }
  
  if (oscillators.empty()) {
    ImGui::Dummy(ImVec2(1.0, 5.0));
    ImGui::Dummy(ImVec2(5.0, 1.0));
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
    ImGui::Text("No active Oscillators");
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(1.0, 5.0));
  }
  
  ImGui::EndChild();
}
