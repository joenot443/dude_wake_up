//
//  TileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "TileBrowserView.hpp"
#include "LibraryService.hpp"
#include "ofxImGui.h"
#include "Strings.hpp"
#include "FontService.hpp"
#include "LibraryFile.hpp"
#include "MarkdownService.hpp"
#include "MarkdownView.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"

static const ImVec2 TileSize = ImVec2(88, 55);

void TileBrowserView::setup(){};

void TileBrowserView::setTileItems(std::vector<std::shared_ptr<TileItem>> items) {
  tileItems = items;
}

void TileBrowserView::sortTileItems() {
  if (tileCount == tileItems.size()) return;
  
  // Count the number of times each category appears in the tileItems
  std::map<std::string, int> categoryCounts;
  for (const auto &tileItem : tileItems) {
    categoryCounts[tileItem->category]++;
  }
  
  // Sort the tileItems based on the number of times each category appears
  std::sort(tileItems.begin(), tileItems.end(),
            [&](const auto &a, const auto &b) {
    // Compare based on category counts
    if (categoryCounts[a->category] != categoryCounts[b->category]) {
      return categoryCounts[a->category] > categoryCounts[b->category];
    }
    if (a->category == b->category) {
      return a->name < b->name;
    }
    // If counts are equal, compare categories alphabetically
    return a->category < b->category;
  });
  
  tileCount = tileItems.size();
}

void TileBrowserView::draw()
{
//  sortTileItems();
  auto n = 0;
  auto size = ImGui::GetContentRegionAvail();
  std::string lastCategory = "";
  for (int idx = 0; idx < tileItems.size(); ++idx)
  {
    bool isLast = tileItems.size() - 1 == idx;
    auto tile = tileItems[idx];
    
    if (lastCategory != tile->category) {
      if (idx != 0) ImGui::NewLine();
      // Add 8.0 of vertical padding
      ImGui::PushFont(FontService::getService()->h4);
      CommonViews::PaddedText(tile->category, ImVec2(2.0, 8.0));
      ImGui::PopFont();
      lastCategory = tile->category;
    }
    
    float xPos = ImGui::GetCursorPosX();
    float yPos = ImGui::GetCursorPosY();
    float endXPos = xPos;

    if (tile->textureID != nullptr)
    {
      bool isLibraryItem = std::dynamic_pointer_cast<LibraryTileItem>(tile) != nullptr;
      
      auto startPos = ImGui::GetCursorPos();
      ImGui::Image(tile->textureID, TileSize);
      ImGui::SameLine();
      
      ImGui::GetWindowDrawList()->AddRectFilled(startPos, ImGui::GetCursorPos(), IM_COL32(0, 0, 0, 178));
      
      endXPos = ImGui::GetCursorPosX();
      
      ImGui::SetCursorPosX(xPos);
      
      ImGui::PushFont(FontService::getService()->p);
      // Set the font color to white
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.6));
      ImGui::Button(tile->name.c_str(), TileSize);
      ImGui::PopStyleColor();
      ImGui::PopFont();
      
      tile->dragCallback();
      
      ImGui::SameLine();
      ImGui::SetCursorPosX(xPos);
      ImGui::SetItemAllowOverlap();
      
      // INFO BUTTON
      
      // If we have a markdown file, draw the info button
      if (MarkdownService::getService()->hasItemForName(tile->name))
      {
        auto popupId = formatString("##%s_tilepopup", tile->name.c_str());
        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr, ImGuiPopupFlags_MouseButtonLeft))
        {
          MarkdownView(tile->name).draw();
          ImGui::EndPopup();
        }
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.5));
        if (CommonViews::IconButton(ICON_MD_INFO, tile->name.c_str()))
        {
          ImGui::OpenPopup(popupId.c_str());
        }
        ImGui::PopStyleColor();
      } else {
        CommonViews::InvisibleIconButton(formatString("##%s_invisible_icon", tile->name.c_str()));
      }
      
      // FAVORITE BUTTON
      if (tile->shaderType != ShaderTypeNone) {
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + TileSize.x - 2 * 16. - 6);
        std::string icon = ParameterService::getService()->isShaderTypeFavorited(tile->shaderType) ?  ICON_MD_FAVORITE : ICON_MD_FAVORITE_BORDER;
        if (CommonViews::IconButton(icon.c_str(), tile->name.c_str()))
        {
          ParameterService::getService()->toggleFavoriteShaderType(tile->shaderType);
        }
      }
      
      
      
      // PROGRESS INDICATOR
      // If we have a LibraryFile as our TileItem, we may need to draw the progress of its download.
      if (isLibraryItem)
      {
        ImGui::SetCursorPosY(yPos);
        ImGui::SetCursorPosX(xPos);
        auto libraryTileItem = std::dynamic_pointer_cast<LibraryTileItem>(tile);
        auto file = libraryTileItem->libraryFile;
        if (LibraryService::getService()->libraryFileIdDownloadedMap[file->id])
        {
          CommonViews::IconTitle(ICON_MD_SAVE);
        }
        else if (file->isDownloading)
        {
          ImGui::Text("%s", formatString("%.0f %", libraryTileItem->libraryFile->progress * 100.0).c_str());
        }
        else
        {
          CommonViews::IconTitle(ICON_MD_CLOUD);
        }
        ImGui::SetItemAllowOverlap();
      }

      ImGui::SetCursorPosY(yPos + TileSize.y - 10);
      ImGui::SetCursorPosX(endXPos);
    }
    else
    {
      if (tile->name.size() > 10)
      {
        // Push a smaller font size
        ImGui::PushFont(FontService::getService()->sm);
        ImGui::Button(tile->name.c_str(), TileSize);
        ImGui::PopFont();
      }
      else
      {
        ImGui::Button(tile->name.c_str(), TileSize);
      }
      tile->dragCallback();
      ImGui::SameLine();
      endXPos = ImGui::GetCursorPosX();
    }

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
};

void TileBrowserView::update(){
  
};
