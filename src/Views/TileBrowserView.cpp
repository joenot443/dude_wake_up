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
//  ImGui::ShowDemoWindow();
  float maxX = ImGui::GetContentRegionAvail().x;
  auto n = 0;
  
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
  ImGui::BeginChild(tileBrowserId.c_str(), ImVec2(maxX, 260), false, window_flags);
  
  
  for (auto tile : tileItems) {
    if (tile.textureID != nullptr) {
      ImGui::ImageButton(tile.textureID, TileSize);
    } else {
      ImGui::Button(tile.name.c_str(), TileSize);
    }
    
    tile.dragCallback();

    float nextX = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x +
                  TileSize.x;

    if (n + 1 < sizeof(tileItems) && nextX < maxX) {
      ImGui::SameLine();
    }
    n += 1;
  }
  
  ImGui::EndChild();
};

void TileBrowserView::update(){

};
