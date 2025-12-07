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
#include "LayoutStateService.hpp"
#include "WaveformOscillator.hpp"
#include "imgui.h"

void OscillatorPanelView::setup() {
  // Setup code if needed
}

void OscillatorPanelView::update() {
  // Update code if needed
}

void OscillatorPanelView::drawOscillatorSettings(std::shared_ptr<Oscillator> oscillator) {
  if (oscillator->type() == OscillatorType_waveform) {
    WaveformOscillator* waveformOscillator = (WaveformOscillator*)oscillator.get();

    CommonViews::ShaderMiniParameter(waveformOscillator->frequency, nullptr);
    CommonViews::ShaderMiniParameter(waveformOscillator->minOutput, nullptr);
    CommonViews::ShaderMiniParameter(waveformOscillator->maxOutput, nullptr);
    CommonViews::ShaderOption(waveformOscillator->waveShape, {
      "Sine Wave", "Square", "Sawtooth", "Triangle",
      "Pulse", "Exp. Sine", "Harmonic", "Rectified",
      "Noise Mod.", "Bitcrush", "Moire"
    }, false);
  }
}

void OscillatorPanelView::draw() {
  auto oscillators = OscillationService::getService()->activeOscillators();
  std::string selectedId = LayoutStateService::getService()->selectedOscillatorSettings;

  ImGui::BeginChild("##oscillatorPanel", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);

  // If an oscillator's settings are selected, only show that one
  if (!selectedId.empty()) {
    // Find the selected oscillator
    std::shared_ptr<Oscillator> selectedOscillator = nullptr;
    for (const auto& oscillator : oscillators) {
      if (oscillator->settingsId == selectedId) {
        selectedOscillator = oscillator;
        break;
      }
    }

    if (selectedOscillator != nullptr && selectedOscillator->enabled->boolValue) {
      ImGui::Dummy(ImVec2(2.0, 5.0));
      ImGui::SameLine();

      ImGui::BeginChild(formatString("##oscillator%s", selectedOscillator->settingsId.c_str()).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize);
      CommonViews::H4Title(formatString("%s - %s", selectedOscillator->value->name.c_str(), selectedOscillator->value->ownerName.c_str()));
      ImVec2 availableSize = ImGui::GetContentRegionAvail();
      ImVec2 oscillatorSize = ImVec2(availableSize.x, 100.0f);

      OscillatorView::draw(selectedOscillator, selectedOscillator->value, oscillatorSize, false);

      // Draw settings below the oscillator
      drawOscillatorSettings(selectedOscillator);

      ImGui::EndChild();
    } else {
      // Oscillator was disabled or not found, clear selection
      LayoutStateService::getService()->selectedOscillatorSettings = "";
    }
  }
  // Otherwise show the full list
  else {
    for (const auto& oscillator : oscillators) {
      if (oscillator->enabled->boolValue) {
        ImGui::Dummy(ImVec2(2.0, 5.0));
        ImGui::SameLine();

        ImGui::BeginChild(formatString("##oscillator%s", oscillator->settingsId.c_str()).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize);
        CommonViews::H4Title(formatString("%s - %s", oscillator->value->name.c_str(), oscillator->value->ownerName.c_str()));
        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        ImVec2 oscillatorSize = ImVec2(availableSize.x, 100.0f);

        OscillatorView::draw(oscillator, oscillator->value, oscillatorSize, false);
        ImGui::EndChild();
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
  }

  ImGui::EndChild();
}
