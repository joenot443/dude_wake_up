//
//  TileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "TileBrowserView.hpp"
#include "ofxImGui.h"
#include "Strings.hpp"
#include "FontService.hpp"
#include "LibraryFile.hpp"
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
    
    if (tile->textureID != nullptr) {
      bool isLibraryItem = std::dynamic_pointer_cast<LibraryTileItem>(tile) != nullptr;
      
      auto startPos = ImGui::GetCursorPos();
      ImGui::Image(tile->textureID, TileSize);
      ImGui::SameLine();
      
      ImGui::GetWindowDrawList()->AddRectFilled(startPos, ImGui::GetCursorPos(), IM_COL32(0, 0, 0, 128));
      
      endXPos = ImGui::GetCursorPosX();

      ImGui::SetCursorPosX(xPos);
      
      
      // TITLE
      if (tile->name.size() > 10) {
        // Push a smaller font size
        ImGui::PushFont(FontService::getService()->sm);
        ImGui::Button(tile->name.c_str(), TileSize);
        ImGui::PopFont();
      } else {
        ImGui::Button(tile->name.c_str(), TileSize);
      }
      
      tile->dragCallback();
      

      ImGui::SameLine();
      ImGui::SetCursorPosX(xPos);
      ImGui::SetItemAllowOverlap();
      
      // INFO BUTTON
      
      // If we have a markdown file, draw the info button
      if (MarkdownService::getService()->hasItemForName(tile->name)) {
        auto popupId = formatString("##%s_tilepopup", tile->name.c_str());
        
        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiPopupFlags_MouseButtonLeft)) {
          MarkdownView(tile->name).draw();
          ImGui::EndPopup();
        }
        if (CommonViews::IconButton(ICON_MD_INFO, tile->name.c_str())) {
          ImGui::OpenPopup(popupId.c_str());
        }
      }
      // PROGRESS INDICATOR
      // If we have a LibraryFile as our TileItem, we may need to draw the progress of its download.
      if (isLibraryItem)  {
        auto libraryTileItem = std::dynamic_pointer_cast<LibraryTileItem>(tile);
        auto file = libraryTileItem->libraryFile;
        if (file->isMediaDownloaded()) {
          ImGui::Text("Saved");
        }
        else if (file->isDownloading) {
          ImGui::Text(formatString("%.0f %", libraryTileItem->libraryFile->progress*100.0).c_str());
        }
        else {
          ImGui::Text("Remote");
        }
      }
      
      
      ImGui::SetCursorPosY(yPos + TileSize.y - 10);
      ImGui::SetCursorPosX(endXPos);
      

      
    } else {
      if (tile->name.size() > 10) {
        // Push a smaller font size
        ImGui::PushFont(FontService::getService()->sm);
        ImGui::Button(tile->name.c_str(), TileSize);
        ImGui::PopFont();
      } else {
        ImGui::Button(tile->name.c_str(), TileSize);
      }
      tile->dragCallback();
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
