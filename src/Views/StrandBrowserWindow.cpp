//
//  StrandBrowserWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/30/24.
//

#include "StrandBrowserWindow.hpp"
#include "StrandTileView.hpp"
#include "imgui.h"

void StrandBrowserWindow::setup() {
  switch (type) {
    case StrandBrowserType_Recent: {
      strands = StrandService::getService()->availableStrands();
      break;
    }
    case StrandBrowserType_Template: {
      strands = StrandService::getService()->availableTemplateStrands();
      break;
    }
  }
}

void StrandBrowserWindow::update() {
  
}
void StrandBrowserWindow::draw()
{
  ImVec2 available = ImGui::GetContentRegionAvail();
  ImGuiStyle& style = ImGui::GetStyle();
  
  // Calculate size for each tile in a 2x2 grid, accounting for spacing
  // Ensure minimum size to avoid division by zero or negative sizes
  float width = (available.x - 36.0 ) / 2;
  float height = (available.y - 18.0 ) / 2;
  
  ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)0), available, ImGuiChildFlags_None, ImGuiWindowFlags_None);

  for (int i = 0; i < strands.size(); ++i)
  {
    if (i == 0) {
      ImGui::Dummy(ImVec2(1.0, 6.0));
    }
    if (i % 2 == 0) {
      ImGui::Dummy(ImVec2(6.0, 6.0));
      ImGui::SameLine();
    }

    // Use unique IDs for child windows
    ImGui::BeginChild(ImGui::GetID((void*)(intptr_t)i), ImVec2(width, height), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
    ImGui::SameLine();
    StrandTileView::draw(strands[i]);
    ImGui::EndChild();
//    ImGui::SameLine();
//    ImGui::Dummy(ImVec2(6.0, 1.0));

    // Place the next item on the same line if it's the first item in a row (i.e., i=0 or i=2)
    // and if there is a next item to draw (i+1 < tiles_to_draw)
    if ((i % 2 == 0) && (i + 1 < strands.size()))
    {
      ImGui::SameLine();
    }
  }
  ImGui::EndChild();
}
