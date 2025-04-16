//
//  BarPlotView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/27/22.
//

#include "BarPlotView.hpp"
#include "implot.h"
#include "Strings.hpp"

void BarPlotView::draw(std::vector<float> yData, std::string id, ImVec2 size) {
  if (yData.size() < 1) { return; }
  
  // Get the max value from data
  float max = 0.0;
  float curr = 0.0;
  std::vector<float> xData = {};

  // Get the max value from data
  for (auto d : yData) {
    if (d > max)
      max = d;
    xData.push_back(curr);
    curr += 1.0;
  }

  max = max * 1.2;
  
  // Draw the plot
  if (ImPlot::BeginPlot(formatString("##%s", id.c_str()).c_str(),
                        size, ImPlotFlags_CanvasOnly)) {
    ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_AutoFit);
    ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_AutoFit);

    ImPlot::PlotBars(formatString("##%s-BarPlot", id.c_str()).c_str(),
                     &xData[0],
                     &yData[0],
                     (int)
                     yData.size(),
                     0.7,
                     ImPlotBarsFlags_None);
    ImPlot::EndPlot();
  }
}
