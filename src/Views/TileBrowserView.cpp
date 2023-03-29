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

static const ImVec2 TileSize = ImVec2(80, 50);

void TileBrowserView::setup(){
};

void TileBrowserView::draw() {
  auto n = 0;
  auto size = ImGui::GetContentRegionAvail();
  
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
      tile.dragCallback();

      ImGui::SameLine();
      ImGui::SetCursorPosX(xPos);
      ImGui::SetItemAllowOverlap();
      
      // If we have a markdown file, draw the info button
      if (MarkdownService::getService()->hasItemForName(tile.name)) {
        auto popupId = formatString("##%s_tilepopup", tile.name.c_str());
        
        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
          MarkdownView(tile.name).draw();
          ImGui::EndPopup();
        }
        if (CommonViews::IconButton(ICON_MD_INFO, tile.name.c_str())) {
          ImGui::OpenPopup(popupId.c_str());
        }
      }
      ImGui::SetCursorPosY(yPos + TileSize.y - 10);
      ImGui::SetCursorPosX(endXPos);
    } else {
      ImGui::Button(tile.name.c_str(), TileSize);
      tile.dragCallback();
      ImGui::SameLine();
      endXPos = ImGui::GetCursorPosX();
    }
    

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
  
  ImGui::NewLine();
};

void TileBrowserView::update(){

};
