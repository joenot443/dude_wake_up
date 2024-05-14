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

void OscillatorView::draw(std::vector<std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>> subjects) {
  if (subjects.empty()) {
    return;
  }
  std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>> first = subjects.front();
  std::shared_ptr<Oscillator> oscillator = std::get<0>(first);
  std::shared_ptr<Parameter> value = std::get<1>(first);
  
  if (oscillator == nullptr)
  {
    return;
  }
  
  OscillatorType type = oscillator->type;
  
  if (oscillator.get()->type == Oscillator_waveform)
  {
    WaveformOscillator *waveformOscillator = (WaveformOscillator *)oscillator.get();
    oscillator->tick();
    ImVector<ImVec2> data = waveformOscillator->data;
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    ImPlot::SetNextAxisLimits(ImAxis_X1, 0.0, waveformOscillator->span);
    ImPlot::SetNextAxisLimits(ImAxis_Y1, waveformOscillator->min(), waveformOscillator->max());
    
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    
    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(200, 200), ImPlotFlags_CanvasOnly))
    {
      ImPlot::PlotLine("", &data[0].x, &data[0].y, data.size(), 0, 5.0, 2 * sizeof(float));
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
  
  else if (type == Oscillator_value)
  {
    ValueOscillator *valueOscillator = (ValueOscillator *)oscillator.get();
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    
    ImPlot::SetNextAxisLimits(ImAxis_X1, valueOscillator->xRange[0], valueOscillator->xRange[1]);
    ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0, 1.0);
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    
    if (ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(200, 200), ImPlotFlags_None))
    {
      for (auto pair : subjects) {
        ValueOscillator *valueOscillator = (ValueOscillator *)std::get<0>(pair).get();
        valueOscillator->tick();
        std::shared_ptr<Parameter> param = std::get<1>(pair);
        std::vector<float> data = valueOscillator->values;
        ImPlot::PlotLine(param->name.c_str(), &data[0], (int)data.size());
      }
      ImPlot::EndPlot();
    }
  }
  else if (type == Oscillator_pulse)
  {
    std::shared_ptr<PulseOscillator> pulseOscillator =  std::dynamic_pointer_cast<PulseOscillator>(oscillator);
    pulseOscillator->tick();
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    
    ImPlot::SetNextAxisLimits(ImAxis_X1, pulseOscillator->xRange[0], pulseOscillator->xRange[1]);
    ImPlot::SetNextAxisToFit(ImAxis_Y1);
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    ImPlot::SetNextAxisLimits(ImAxis_X1, pulseOscillator->xRange[0], pulseOscillator->xRange[1]);
    bool size = oscillator->values().size() > 0;
    if (size && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(200, 200), ImPlotFlags_CanvasOnly))
    {
      std::vector<float> values = pulseOscillator->values();
      ImPlot::PlotLine(value->name.c_str(), &values.at(0), (int)values.size());
      ImPlot::EndPlot();
    }
  }
}

void OscillatorView::draw(std::shared_ptr<Oscillator> oscillator,
                          std::shared_ptr<Parameter> value)
{
  std::vector<
  std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>> subjects =
  std::vector<
  std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>>
  (1, std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>(oscillator, value));
  draw(subjects);
}

void OscillatorView::drawMini(std::shared_ptr<Oscillator> oscillator, std::shared_ptr<Parameter> value) {
  if (oscillator.get()->type == Oscillator_waveform)
  {
    WaveformOscillator *waveformOscillator = (WaveformOscillator *)oscillator.get();
    oscillator->tick();
    ImVector<ImVec2> data = waveformOscillator->data;
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    ImPlot::SetNextAxisLimits(ImAxis_X1, 0.0, waveformOscillator->span);
    ImPlot::SetNextAxisLimits(ImAxis_Y1, waveformOscillator->min(), waveformOscillator->max());
    
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.0f, 0.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0,0));

    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), ImVec2(50, 50), ImPlotFlags_CanvasOnly | ImPlotFlags_NoFrame))
    {
      ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxisFormat(ImAxis_X1, "");
      ImPlot::SetupAxisFormat(ImAxis_Y1, "");
      ImPlot::SetupLegend(ImPlotLocation_SouthWest);
      ImPlot::PlotLine("", &data[0].x, &data[0].y, data.size(), 0, 5.0, 2 * sizeof(float));
      ImPlot::EndPlot();
    }
    
    ImGui::SameLine(0, 5);
    ImGui::VSliderFloat(formatString("##freq%s", value->name.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->frequency->value, 0.0f,
                        100.0f, "F", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##amp%s", value->name.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->amplitude->value, 0.0f,
                        waveformOscillator->amplitude->max, "A",
                        ImGuiSliderFlags_None);
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##shift%s", value->name.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->shift->value,
                        -value->max * 2, value->max * 2, "S");
    ImGui::PushFont(FontService::getService()->sm);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50);
    ImGui::Text("%s", formatString("%.2f", waveformOscillator->frequency->value).c_str());
    ImGui::SameLine(0, 5);
    ImGui::Text("%s", formatString("%.2f", waveformOscillator->amplitude->value).c_str());
    ImGui::SameLine(0, 5);
    ImGui::Text("%s", formatString("%.2f", waveformOscillator->shift->value).c_str());
    ImGui::PopFont();
  }
}
