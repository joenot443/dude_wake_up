//
//  Waveform.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "OscillatorView.hpp"
#include "CommonViews.hpp"
#include "Video.hpp"
#include "FontService.hpp"
#include "Fonts.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "Colors.hpp"
#include "LayoutStateService.hpp"
#include "PulseOscillator.hpp"
#include "WaveformOscillator.hpp"
#include "Strings.hpp"
#include "ValueOscillator.hpp"
#include "implot.h"
#include "ofxImGui.h"


void OscillatorView::draw(std::vector<std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>> subjects, ImVec2 size, bool drawExtras) {
  
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
  
  OscillatorType type = oscillator->type();
  
  if (type == OscillatorType_waveform)
  {
    WaveformOscillator *waveformOscillator = (WaveformOscillator *)oscillator.get();
    ImVector<ImVec2> data = waveformOscillator->data;

    // Calculate sizes to make plot hug the buttons
    float buttonWidth = 50.0f;
    float buttonSpacing = 10.0f;
    ImVec2 plotSize = ImVec2(size.x - buttonWidth - buttonSpacing, size.y);

    // Set x-axis limits based on the current time and data size
    float currentTime = frameTime() + 2.0;
    float startTime = fmax(currentTime - 10.0, 0.0f);

    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    ImPlot::SetNextAxesToFit();

    // Draw the plot with calculated size
    if (data.size() > 0 && ImPlot::BeginPlot(formatString("##plot%s", value->paramId.c_str()).c_str(), plotSize, ImPlotFlags_CanvasOnly | (drawExtras ? 0 : ImPlotFlags_NoLegend)))
    {
      if (!drawExtras) {
        ImPlot::SetupAxis(ImAxis_X1, NULL, ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxis(ImAxis_Y1, NULL, ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxisFormat(ImAxis_X1, "");
        ImPlot::SetupAxisFormat(ImAxis_Y1, "");
      }

      ImPlot::PlotLine("##", &data[0].x, &data[0].y, data.size(), ImPlotLineFlags_Segments, 2.0, 2 * sizeof(float));

      // Add plotting for min/max lines if enabled
      if (waveformOscillator->showMinMax) {
        float oscMin = waveformOscillator->min();
        float oscMax = waveformOscillator->max();
        float paramMin = value->min;
        float paramMax = value->max;

        // Check if the oscillator's range is within the parameter's bounds
        bool isInBounds = (oscMin >= paramMin) && (oscMax <= paramMax);

        // Define colors
        ImVec4 lineColor = isInBounds ? ImVec4(0.4f, 1.0f, 0.4f, 0.4f) // Soft green
                                      : ImVec4(1.0f, 0.4f, 0.4f, 0.4f); // Soft red

        ImPlot::PushStyleColor(ImPlotCol_Line, lineColor);
        ImPlot::PlotInfLines("##MinLine", &oscMin, 1, ImPlotInfLinesFlags_Horizontal);
        ImPlot::PlotInfLines("##MaxLine", &oscMax, 1, ImPlotInfLinesFlags_Horizontal);
        ImPlot::PopStyleColor();
      }

      ImPlot::EndPlot();
    }

    ImGui::SameLine(0, buttonSpacing);

    // Draw 3 buttons in a column, vertically centered
    ImGui::BeginChild(formatString("##child%s", value->paramId.c_str()).c_str(), ImVec2(buttonWidth, plotSize.y), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoScrollbar);

    // Center the buttons vertically
    ImVec2 buttonSize = ImVec2(25.0f, 25.0f);
    ImVec2 buttonPad = ImVec2(2.0, 2.0);
    float spacing = 8.0f;

    // Play/Pause button
    const char* playPauseImage = waveformOscillator->enabled->boolValue ? "pause.png" : "play.png";
    if (CommonViews::ImageButton(formatString("playPause%s", value->paramId.c_str()), playPauseImage, buttonSize, buttonPad)) {
      waveformOscillator->enabled->setBoolValue(!waveformOscillator->enabled->boolValue);
    }

    // Settings button
    const char* settingsImage = "settings.png";
    if (CommonViews::ImageButton(formatString("settings%s", value->paramId.c_str()), settingsImage, buttonSize, buttonPad)) {
      // Toggle the selected oscillator settings
      if (LayoutStateService::getService()->selectedOscillatorSettings == oscillator->settingsId) {
        LayoutStateService::getService()->selectedOscillatorSettings = "";
      } else {
        LayoutStateService::getService()->selectedOscillatorSettings = oscillator->settingsId;
      }
    }

    // Reset button
    const char* resetImage = "undo.png";
    if (CommonViews::ImageButton(formatString("reset%s", value->paramId.c_str()), resetImage, buttonSize, buttonPad)) {
      waveformOscillator->minOutput->resetValue();
      waveformOscillator->maxOutput->resetValue();
      waveformOscillator->frequency->resetValue();
    }
    ImGui::EndChild();
  }
  
  else if (type == OscillatorType_value)
  {
    ValueOscillator *valueOscillator = (ValueOscillator *)oscillator.get();
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    
    ImPlot::SetNextAxisLimits(ImAxis_X1, valueOscillator->xRange[0], valueOscillator->xRange[1]);
    ImPlot::SetNextAxisLimits(ImAxis_Y1, 0.0, 1.0);
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, Colors::EditorBackgroundColor.Value);
    if (ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), size, ImPlotFlags_None | (drawExtras ? 0 : ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)))
    {
      if (!drawExtras) {
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxisFormat(ImAxis_X1, "");
        ImPlot::SetupAxisFormat(ImAxis_Y1, "");
      }
      int idx = 0;
      for (auto pair : subjects) {
        ValueOscillator *valueOscillator = (ValueOscillator *)std::get<0>(pair).get();
        valueOscillator->tick();
        std::shared_ptr<Parameter> param = std::get<1>(pair);
        std::vector<float> data = valueOscillator->values;
        ImPlot::PlotLine(drawExtras ? param->name.c_str() : formatString("##%d", idx).c_str(), &data[0], (int)data.size());
        idx += 1;
      }
      ImPlot::EndPlot();
    }
  }
  else if (type == OscillatorType_pulse)
  {
    std::shared_ptr<PulseOscillator> pulseOscillator =  std::dynamic_pointer_cast<PulseOscillator>(oscillator);
    pulseOscillator->tick();
    
    static int rt_axis = ImPlotAxisFlags_AuxDefault & ~ImPlotAxisFlags_NoTickLabels;
    
    ImPlot::SetNextAxisLimits(ImAxis_X1, pulseOscillator->xRange[0], pulseOscillator->xRange[1]);
    ImPlot::SetNextAxisToFit(ImAxis_Y1);
    auto plotStyle = ImPlot::GetStyle();
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(0.3f, 0.3f));
    bool hasValues = oscillator->values().size() > 0;
    if (hasValues && ImPlot::BeginPlot(formatString("##plot%s", value->name.c_str()).c_str(), size, ImPlotFlags_CanvasOnly | (drawExtras ? 0 : ImPlotFlags_CanvasOnly)))
    {
      if (!drawExtras) {
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel);
        ImPlot::SetupAxisFormat(ImAxis_X1, "");
        ImPlot::SetupAxisFormat(ImAxis_Y1, "");
      }
      std::vector<float> values = pulseOscillator->values();
      ImPlot::PlotLine(drawExtras ? value->name.c_str() : "##", &values.at(0), (int)values.size());
      ImPlot::EndPlot();
    }
  }
}

void OscillatorView::draw(std::shared_ptr<Oscillator> oscillator,
                          std::shared_ptr<Parameter> value,
                          ImVec2 size,
                          bool drawExtras) {
  std::vector<
  std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>> subjects =
  std::vector<
  std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>>
  (1, std::tuple<std::shared_ptr<Oscillator>, std::shared_ptr<Parameter>>(oscillator, value));
  draw(subjects, size, drawExtras);
}

void OscillatorView::drawMini(std::shared_ptr<Oscillator> oscillator, std::shared_ptr<Parameter> value, bool drawExtras) {
  if (oscillator.get()->type() == OscillatorType_waveform)
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
      ImPlot::PlotLine("", &data[0].x, &data[0].y, data.size(), ImPlotLineFlags_NoClip, 5.0, 2 * sizeof(float));

      // Add plotting for min/max lines if enabled in drawMini
      if (waveformOscillator->showMinMax) {
        float oscMin = waveformOscillator->min();
        float oscMax = waveformOscillator->max();
        float paramMin = value->min;
        float paramMax = value->max;

        // Check if the oscillator's range is within the parameter's bounds
        bool isInBounds = (oscMin >= paramMin) && (oscMax <= paramMax);

        // Define colors
        ImVec4 lineColor = isInBounds ? ImVec4(0.4f, 1.0f, 0.4f, 0.4f) // Soft green
                                      : ImVec4(1.0f, 0.4f, 0.4f, 0.4f); // Soft red

        ImPlot::PushStyleColor(ImPlotCol_Line, lineColor);
        ImPlot::PlotInfLines("##MinLineMini", &oscMin, 1, ImPlotInfLinesFlags_Horizontal);
        ImPlot::PlotInfLines("##MaxLineMini", &oscMax, 1, ImPlotInfLinesFlags_Horizontal);
        ImPlot::PopStyleColor();
      }

      ImPlot::EndPlot();
    }
    
    ImGui::SameLine(0, 5);
    ImGui::GetWindowDrawList()->AddRect(ImGui::GetCursorPos() - ImVec2(2, 2), ImVec2(ImGui::GetCursorPos().x + 20, ImGui::GetCursorPos().y + 55), ImColor(255, 0, 0), 4.0, ImDrawFlags_RoundCornersAll, 2.0);
    ImGui::BeginChild(formatString("##freqChild%s", value->name.c_str()).c_str(), ImVec2(0,0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoDecoration);
    CommonViews::ResetButton("freqReset", waveformOscillator->frequency);
    ImGui::VSliderFloat(formatString("##freq%s%s", value->name.c_str(), value->paramId.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->frequency->value, 0.0f,
                        100.0f, "F", ImGuiSliderFlags_Logarithmic);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##amp%s%s", value->name.c_str(), value->paramId.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->minOutput->value, 0.0f,
                        waveformOscillator->maxOutput->max, "A",
                        ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine();
    ImGui::VSliderFloat(formatString("##shift%s%s", value->name.c_str(), value->paramId.c_str()).c_str(),
                        ImVec2(15, 50), &waveformOscillator->minOutput->value,
                        -value->max * 2, value->max * 2, "S", ImGuiSliderFlags_Logarithmic);

    if (drawExtras) {
      ImGui::PushFont(FontService::getService()->current->sm);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50);
      ImGui::Text("%s", formatString("%.2f", waveformOscillator->frequency->value).c_str());
      ImGui::SameLine(0, 5);
      ImGui::Text("%s", formatString("%.2f", waveformOscillator->minOutput->value).c_str());
      ImGui::SameLine(0, 5);
      ImGui::Text("%s", formatString("%.2f", waveformOscillator->maxOutput->value).c_str());
      ImGui::PopFont();
    }
  }
}
