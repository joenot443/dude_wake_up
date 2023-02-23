//
//  Waveform.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "OscillatorView.hpp"
#include "CommonViews.hpp"
#include "FontService.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "PulseOscillator.hpp"
#include "WaveformOscillator.hpp"
#include "Strings.hpp"
#include "ValueOscillator.hpp"
#include "implot.h"
#include "ofxImGui.h"

void OscillatorView::draw(std::shared_ptr<Oscillator> oscillator,
                          std::shared_ptr<Parameter> value) {
  if (oscillator == nullptr) {
    return;
  }
  
  OscillatorType type = oscillator->type;
  
  if (oscillator.get()->type == Oscillator_waveform) {
    WaveformOscillator* waveformOscillator = (WaveformOscillator *) oscillator.get();
    oscillator->tick();
    ImVector<ImVec2> data = waveformOscillator->data;
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    ImPlot::SetNextAxisLimits(ImAxis_X1, 0.0, waveformOscillator->span);
    ImPlot::SetNextAxisToFit(ImAxis_Y1);

    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    
    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(200, 200), ImPlotFlags_CanvasOnly)) {
      ImPlot::PlotLine("", &data[0].x, &data[0].y, data.size(), 0, 5.0, 2*sizeof(float));
      ImPlot::EndPlot();
    }
    ImGui::SameLine(0, 10);
    ImGui::VSliderFloat(formatString("##freq%s", value->name.c_str()).c_str(),
                        ImVec2(40, 160), &waveformOscillator->frequency->value, 0.0f,
                        100.0f, "Freq.\n%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##amp%s", value->name.c_str()).c_str(),
                        ImVec2(40, 160), &waveformOscillator->amplitude->value, 0.0f,
                        waveformOscillator->amplitude->max, "Amp.\n%.2f",
                        ImGuiSliderFlags_None);
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##shift%s", value->name.c_str()).c_str(),
                        ImVec2(40, 160), &waveformOscillator->shift->value,
                        -value->max * 2, value->max * 2, "Shift\n%.2f");
  }
  
  else if (type == Oscillator_value) {
    oscillator->tick();
    ValueOscillator* valueOscillator = (ValueOscillator *) oscillator.get();
    std::vector<float> data = valueOscillator->values;
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    
    ImPlot::SetNextAxisLimits(ImAxis_X1, valueOscillator->xRange[0], valueOscillator->xRange[1]);
    ImPlot::SetNextAxisToFit(ImAxis_Y1);
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    
    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(200, 200), ImPlotFlags_CanvasOnly)) {
      ImPlot::PlotLine(value->name.c_str(), &data[0], (int) data.size());
      ImPlot::EndPlot();
    }
  }
}
