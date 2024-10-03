//
//  BarPlotView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/27/22.
//

#include "BarPlotView.hpp"
#include "implot.h"
#include "Strings.hpp"

void BarPlotView::draw(std::vector<float> yData, std::string id) {
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
  
  if (max < 0.01) {
    ImGui::BeginChildFrame(123, ImVec2(200,200));
    ImGui::EndChildFrame();
  }
  
  // Draw the plot
  if (ImPlot::BeginPlot(formatString("##%s", id.c_str()).c_str(),
                        ImVec2(200, 200), ImPlotFlags_CanvasOnly)) {
    ImPlot::SetupAxesLimits(0.0, 1.0 * yData.size(), 0.0, max);
    ImPlot::PlotBars(formatString("##%s", id.c_str()).c_str(),
                     &xData[0],
                     &yData[0],
                     (int)
                     yData.size(),
                     0.7,
                     1.0);
    ImPlot::EndPlot();
  }
}
