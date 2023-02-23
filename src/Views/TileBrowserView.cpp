//
//  TileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "TileBrowserView.hpp"
#include "ofxImGui.h"
#include "Strings.hpp"
#include "MarkdownService.hpp"
#include "MarkdownView.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"

static const ImVec2 TileSize = ImVec2(100, 70);

void TileBrowserView::setup(){
};

void TileBrowserView::draw() {
  auto n = 0;
  auto size = ImGui::GetContentRegionAvail();
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
  ImGui::BeginChild(tileBrowserId.c_str(), size, false, window_flags);
  
  
  for (auto tile : tileItems) {
    float xPos = ImGui::GetCursorPosX();
    float yPos = ImGui::GetCursorPosY();
    float endXPos = xPos;
    
    if (tile.textureID != nullptr) {
      ImGui::Image(tile.textureID, TileSize);
      ImGui::SameLine();
      
      endXPos = ImGui::GetCursorPosX();

      ImGui::SetCursorPosX(xPos);
      ImGui::Button(tile.name.c_str(), TileSize);
      
      ImGui::SameLine();
      ImGui::SetCursorPosX(xPos);
      ImGui::SetItemAllowOverlap();
      
      // If we have a markdown file, draw the info button
      if (MarkdownService::getService()->hasItemForName(tile.name)) {
        auto popupId = formatString("##%s_tilepopup");
        
        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
          MarkdownView(tile.name).draw();
          ImGui::EndPopup();
        }
        
        if (CommonViews::IconButton(ICON_MD_INFO, tile.name.c_str())) {
          ImGui::OpenPopup(popupId.c_str());
        }
      }

      
      
      
      ImGui::SetCursorPosX(endXPos);
    } else {
      ImGui::Button(tile.name.c_str(), TileSize);
      ImGui::SameLine();
      endXPos = ImGui::GetCursorPosX();
    }
    
    tile.dragCallback();

    float nextX = ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x +
                  TileSize.x;

    if (n + 1 < sizeof(tileItems) && nextX < size.x) {
      ImGui::SameLine();
      ImGui::SetCursorPosX(endXPos);
    } else {
      ImGui::NewLine();
    }
    n += 1;
  }
  
  ImGui::EndChild();
};

void TileBrowserView::update(){

};
