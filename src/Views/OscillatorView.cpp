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
#include "implot.h"
#include "Strings.h"

void OscillatorView::draw(const char* name, Parameter* value, ValueOscillator *oscillator) {
  if (oscillator->enabled) {
    ImGui::PushFont(FontService::getService()->h4);
    ImGui::Text("%s Oscillator", name);
    ImGui::Spacing();
    ImGui::PopFont();
    ImGui::VSliderFloat(formatString("##amp%s", name).c_str(), ImVec2(40,160), &oscillator->amplitude, 0.0f, 20.0f, "Amp.\n%.2f");
    ImGui::SameLine(0, 20);
    ImGui::VSliderFloat(formatString("##shift%s", name).c_str(), ImVec2(40,160), &oscillator->shift, -10.0f, 10.0f, "Shift\n%.2f");
    ImGui::SameLine(0, 20);
    oscillator->tick();
    ImVector<ImVec2> data = oscillator->data;
    static int rt_axis = ImPlotAxisFlags_Default & ~ImPlotAxisFlags_TickLabels;
    ImPlot::SetNextPlotLimitsX(oscillator->xRange[0], oscillator->xRange[1], ImGuiCond_Always);
    ImPlot::SetNextPlotLimitsY(oscillator->yRange[0], oscillator->yRange[1], ImGuiCond_Always);
    if (ImPlot::BeginPlot(formatString("##plot%s", name).c_str(), NULL, NULL, ImVec2(-1,150), ImPlotFlags_Default, rt_axis, rt_axis)) {
        ImPlot::PlotLine(name, &data[0].x, &data[0].y, data.size(), 0, 2 * sizeof(float));
        ImPlot::EndPlot();
    }
    
    value->value = oscillator->value;
  }
}

void OscillatorView::draw(const char* name, Parameter* value, Oscillator *oscillator) {
  if (oscillator->enabled) {
    ImGui::PushFont(FontService::getService()->h4);
    ImGui::Text("%s Oscillator", name);
    ImGui::Spacing();
    ImGui::PopFont();
    ImGui::VSliderFloat(formatString("##freq%s", name).c_str(), ImVec2(40,160), &oscillator->frequency, 0.0f, 2.0f, "Freq.\n%.2f", ImGuiSliderFlags_None);
    ImGui::SameLine(0, 10);
    ImGui::VSliderFloat(formatString("##amp%s", name).c_str(), ImVec2(40,160), &oscillator->amplitude, value->min, value->max, "Amp.\n%.2f", ImGuiSliderFlags_None);
    ImGui::SameLine(0, 20);
    ImGui::VSliderFloat(formatString("##shift%s", name).c_str(), ImVec2(40,160), &oscillator->shift, -5.0f, 5.0f, "Shift\n%.2f", ImGuiSliderFlags_None);
    ImGui::SameLine(0, 20);
    oscillator->tick();
    ImVector<ImVec2> data = oscillator->data;
    static int rt_axis = ImPlotAxisFlags_Default & ~ImPlotAxisFlags_TickLabels;
    ImPlot::SetNextPlotLimitsX(oscillator->xRange[0], oscillator->xRange[1], ImGuiCond_Always);
    ImPlot::SetNextPlotLimitsY(oscillator->yRange[0], oscillator->yRange[1], ImGuiCond_Always);
    if (ImPlot::BeginPlot(formatString("##plot%s", name).c_str(), NULL, NULL, ImVec2(-1,150), ImPlotFlags_Default, rt_axis, rt_axis)) {
        ImPlot::PlotLine(name, &data[0].x, &data[0].y, data.size(), 0, 2 * sizeof(float));
        ImPlot::EndPlot();
    }
    
    value->value = oscillator->value->value;
  }
}



