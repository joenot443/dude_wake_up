//
//  Waveform.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "OscillatorView.hpp"
#include "ofxImGui.h"
#include "FontService.hpp"
#include "ValueOscillator.hpp"
#include "CommonViews.hpp"
#include "implot.h"
#include "Strings.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "PulseOscillator.hpp"


void OscillatorView::setup() {
}

void OscillatorView::update() {
}

void OscillatorView::draw() {
  auto oscillator = OscillationService::getService()->selectedOscillator;
  auto value = ParameterService::getService()->selectedParameter;
  
  if (oscillator != nullptr) {
    ImGui::PushFont(FontService::getService()->h4);
    ImGui::Text("%s Oscillator", value->name.c_str());
    ImGui::Spacing();
    ImGui::PopFont();
    ImGui::VSliderFloat(formatString("##freq%s", value->name.c_str()).c_str(), ImVec2(40,160), &oscillator->frequency->value, 0.0f, 100.0f, "Freq.\n%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine(0, 10);
    ImGui::VSliderFloat(formatString("##amp%s", value->name.c_str()).c_str(), ImVec2(40,160), &oscillator->amplitude->value, 0.0f, oscillator->amplitude->max, "Amp.\n%.2f", ImGuiSliderFlags_None);
    ImGui::SameLine(0, 20);
    ImGui::VSliderFloat(formatString("##shift%s", value->name.c_str()).c_str(), ImVec2(40,160), &oscillator->shift->value, -value->max * 2, value->max * 2, "Shift\n%.2f");
    ImGui::SameLine(0, 20);
    oscillator->tick();
    ImVector<ImVec2> data = oscillator->data;
    static int rt_axis = ImPlotAxisFlags_Default & ~ImPlotAxisFlags_TickLabels;
    ImPlot::SetNextPlotLimitsX(oscillator->xRange[0], oscillator->xRange[1], ImGuiCond_Always);
    ImPlot::SetNextPlotLimitsY(oscillator->yRange[0], oscillator->yRange[1], ImGuiCond_Always);
    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), NULL, NULL, ImVec2(-1,150), ImPlotFlags_Default, rt_axis, rt_axis)) {
      ImPlot::PlotLine(value->name.c_str(), &data[0].x, &data[0].y, data.size(), 0, 2 * sizeof(float));
      ImPlot::EndPlot();
    }
  }
}
