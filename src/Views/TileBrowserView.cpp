//
//  TileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "TileBrowserView.hpp"
#include "ofxImGui.h"

static const ImVec2 TileSize = ImVec2(120, 80);

void TileBrowserView::setup(){
};

void TileBrowserView::draw() {
  auto n = 0;
  auto size = ImGui::GetContentRegionAvail();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
  ImGui::BeginChild(tileBrowserId.c_str(), size, false, window_flags);
  
  
  for (auto tile : tileItems) {
    if (tile.textureID != nullptr) {
      float xPos = ImGui::GetCursorPosX();
      ImGui::Image(tile.textureID, TileSize);
      ImGui::SetItemAllowOverlap();
      ImGui::SameLine();
      ImGui::SetCursorPosX(xPos);
      ImGui::Button(tile.name.c_str(), TileSize);
    } else {
      ImGui::Button(tile.name.c_str(), TileSize);
    }
    
    tile.dragCallback();

    float nextX = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x +
                  TileSize.x;

    if (n + 1 < sizeof(tileItems) && nextX < size.x) {
      ImGui::SameLine();
    }
    n += 1;
  }
  
  ImGui::EndChild();
};

void TileBrowserView::update(){

};
