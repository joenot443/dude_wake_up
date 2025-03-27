//
//  ParameterTileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/16/24.
//

#include "ParameterTileBrowserView.hpp"
#include "ofxImGui.h"
#include "CommonViews.hpp"
#include "FontService.hpp"

static const ImVec2 TileSize = ImVec2(75, 75);


bool  ParameterTileBrowserView::draw(std::vector<std::shared_ptr<ParameterTileItem>> tileItems, std::shared_ptr<Parameter> parameter, bool drawNames) {
  bool ret = false;
  auto n = 0;
  auto size = ImGui::GetContentRegionAvail();
  
  // Sort the tile items by category
  std::vector<std::shared_ptr<ParameterTileItem>> sortedItems;
  for (auto item : tileItems)
  {
    sortedItems.push_back(item);
  }
  std::sort(sortedItems.begin(), sortedItems.end(), [](std::shared_ptr<ParameterTileItem> a, std::shared_ptr<ParameterTileItem> b)
            { return a->category < b->category; });
  std::string lastCategory = "";
  for (int idx = 0; idx < sortedItems.size(); ++idx)
  {
    bool isLast = sortedItems.size() - 1 == idx;
    auto tile = sortedItems[idx];
    
    if (lastCategory != tile->category) {
      CommonViews::H4Title(tile->category);
      lastCategory = tile->category;
    }
    
    float xPos = ImGui::GetCursorPosX();
    float yPos = ImGui::GetCursorPosY();
    float endXPos = xPos;
    
    if (tile->textureID != 0)
    {
      auto startPos = ImGui::GetCursorPos();
      ImGui::Image(tile->textureID, TileSize);
      ImGui::SameLine();
      
      ImGui::GetWindowDrawList()->AddRectFilled(startPos, ImGui::GetCursorPos(), IM_COL32(255, 255, 255, 128));
      
      endXPos = ImGui::GetCursorPosX();
      
      ImGui::SetCursorPosX(xPos);
    }
    
    // TITLE
    // Push a smaller font size
    ImGui::PushFont(FontService::getService()->sm);
    std::string buttonTitle = drawNames ? tile->name : "##" + tile->name;
    if (ImGui::Button(buttonTitle.c_str(), TileSize)) {
      ret = true;
      parameter->setValue(static_cast<float>(idx));
    }
    ImGui::PopFont();
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(xPos);
    ImGui::SetItemAllowOverlap();
    float nextX = ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x + TileSize.x;
    
    if (n + 1 < tileItems.size() && nextX < size.x)
    {
      ImGui::SameLine();
      ImGui::SetCursorPosX(endXPos);
    }
    else
    {
      ImGui::NewLine();
    }
    n += 1;
  }
  
  ImGui::NewLine();
  return ret;
};
