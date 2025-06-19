//
//  TileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "TileBrowserView.hpp"
#include "LibraryService.hpp"
#include "imgui.h"
#include "ofxImGui.h"
#include "CreditView.hpp"
#include "Strings.hpp"
#include "FontService.hpp"
#include "LibraryFile.hpp"
#include "MarkdownService.hpp"
#include "MarkdownView.hpp"
#include "Colors.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"
#include <filesystem>

void TileBrowserView::setup() {}

void TileBrowserView::setTileItems(std::vector<std::shared_ptr<TileItem>> items) {
  // Clear existing data
  categoryMap.clear();
  categories.clear();
  
  // Group items by category
  for (const auto& item : items) {
    categoryMap[item->category].push_back(item);
    
    // Add category to ordered list if it's new
    if (std::find(categories.begin(), categories.end(), item->category) == categories.end()) {
      categories.push_back(item->category);
    }
  }
  
  // Sort categories by number of items
  std::sort(categories.begin(), categories.end(),
            [this](const std::string& a, const std::string& b) {
    return categoryMap[a].size() > categoryMap[b].size();
  });
  
  // Sort items within each category by name
  for (auto& [category, categoryItems] : categoryMap) {
    std::sort(categoryItems.begin(), categoryItems.end(),
              [](const auto& a, const auto& b) {
      return a->name < b->name;
    });
  }
  
  if (categories.size() > 0) {
    activeCategory = categories.front();
  }
}

void TileBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  tileClickCallback = callback;
}

void TileBrowserView::applyHeaderStyles(bool isSelected) {
  if (isSelected) {
    ImGui::PushStyleColor(ImGuiCol_Header, Colors::ButtonSelected.Value);
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::NodeBorderColor.Value);
  }
}

void TileBrowserView::popHeaderStyles(bool isSelected) {
  ImGui::PopStyleColor(isSelected ? 2 : 0);
}

void TileBrowserView::setContextMenuItems(const std::vector<TileContextMenuItem> &items) {
  contextMenuItems = items;
}

void TileBrowserView::draw() {
  auto availableWidth = ImGui::GetContentRegionAvail().x - 4.0;
  float spacing = ImGui::GetStyle().ItemSpacing.x;
  
  // Calculate tile width: (available width - 2 * spacing) / 3 tiles
  float tileWidth = (availableWidth - (2 * spacing)) / 3.0f;
  // Maintain aspect ratio of original tiles (88:55)
  float tileHeight = tileWidth * (55.0f/88.0f);
  ImVec2 tileSize(tileWidth, tileHeight);
  
  if (categories.size() == 1 || singleCategory) {
    drawSingleCategory(categories[0], tileSize);
  } else {
    drawCategories(tileSize);
  }
  
  ImGui::NewLine();
}

void TileBrowserView::drawSingleCategory(const std::string& category, const ImVec2& tileSize) {
  // Start a new group to contain the tiles
  ImGui::BeginGroup();
  int tileCount = 0;
  
  std::vector<std::shared_ptr<TileItem>> tiles = categoryMap[category];
  
  for (const auto& tile : tiles) {
    // If we've drawn 3 tiles, start a new row
    if (tileCount > 0 && tileCount % 3 == 0) {
      ImGui::NewLine();
    }
    
    drawTile(tile, tileSize);
    tileCount++;
  }
  
  ImGui::EndGroup();
  ImGui::NewLine();
}

void TileBrowserView::drawCategories(const ImVec2& tileSize) {
  // Draw each category as a collapsable header
  for (const auto& category : categories) {
    ImGui::PushFont(FontService::getService()->current->h4);
    
    bool isOpen = activeCategory == category;
    ImGui::SetNextItemOpen(isOpen);
    
    applyHeaderStyles(isOpen);
    if (ImGui::CollapsingHeader(category.c_str())) {
      if (!isOpen) {
        activeCategory = category;
      }
    } else if (isOpen) {
      activeCategory = "";
    }
    popHeaderStyles(isOpen);
    ImGui::PopFont();
    
    // Draw tiles for active category
    if (activeCategory == category) {
      drawSingleCategory(category, tileSize);
    }
  }
}

void TileBrowserView::drawTile(std::shared_ptr<TileItem> tile, const ImVec2& tileSize) {
  std::string childId = formatString("##tile_%s_%s", tile->id.c_str(), tileBrowserId.c_str());

  if (tile->textureID != 0) {
    // Store initial cursor position
    ImVec2 startPos = ImGui::GetCursorScreenPos();

    // Create a child frame to contain everything
    ImGui::BeginChild(childId.c_str(), tileSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration);
    
    ImGui::SetNextItemAllowOverlap();
    
    // Draw the base image
    ImGui::Image(tile->textureID, tileSize);
    
    // Draw dark overlay
    ImGui::GetWindowDrawList()->AddRectFilled(
      startPos,
      ImVec2(startPos.x + tileSize.x, startPos.y + tileSize.y),
      IM_COL32(0, 0, 0, 50)
    );
    
    // Draw the clickable button with same size as image
    ImGui::SetCursorScreenPos(startPos);
    ImGui::PushFont(FontService::getService()->current->p);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.6));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2));
    ImGui::SetNextItemAllowOverlap();
    ImGui::PushID(tile->id.c_str());
    if (ImGui::Button(idAppendedToString(tile->name.c_str(), tile->id).c_str(), tileSize)) {
      if (tileClickCallback != NULL)
      	tileClickCallback(tile);
    }
    ImGui::PopID();
    ImGui::PopStyleColor(4);
    ImGui::PopFont();
    
    if (tile->dragCallback != NULL) {
      tile->dragCallback(tile->id);
    }
    
    // Draw info button in top-left
    ImGui::SetCursorScreenPos(ImVec2(startPos.x + 4, startPos.y + 4));
    if (LibraryService::getService()->hasCredit(tile->shaderType)) {
      auto popupId = formatString("##%s_tilepopup", tile->id.c_str());
      CreditView::draw(popupId, LibraryService::getService()->getShaderCredit(tile->shaderType));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.5));
      if (CommonViews::IconButton(ICON_MD_INFO, tile->id.c_str())) {
        ImGui::OpenPopup(popupId.c_str());
      }
      ImGui::PopStyleColor();
    } else {
      CommonViews::InvisibleIconButton(formatString("##%s_invisible_icon", tile->id.c_str()));
    }
    
    ImGui::SetNextItemAllowOverlap();
    
    // Draw favorite button in top-right
    if (tile->tileType == TileType_Shader) {
      ImGui::SetCursorScreenPos(ImVec2(startPos.x + tileSize.x - 24, startPos.y + 4));
      std::string icon = ParameterService::getService()->isShaderTypeFavorited(tile->shaderType) ? 
        ICON_MD_FAVORITE : ICON_MD_FAVORITE_BORDER;
      if (CommonViews::IconButton(icon.c_str(), tile->name.c_str())) {
        ParameterService::getService()->toggleFavoriteShaderType(tile->shaderType);
      }
    }
    
    // Draw progress indicator in top-left
    if (tile->tileType == TileType_Library) {
      ImGui::SetCursorScreenPos(ImVec2(startPos.x + 4, startPos.y + 4));
      auto libraryTileItem = std::dynamic_pointer_cast<LibraryTileItem>(tile);
      auto file = libraryTileItem->libraryFile;
      if (LibraryService::getService()->libraryFileIdDownloadedMap[file->id]) {
        CommonViews::IconTitle(ICON_MD_SAVE);
      } else if (file->isDownloading) {
        ImGui::Text("%s", formatString("%.0f %", libraryTileItem->libraryFile->progress * 100.0).c_str());
      } else {
        CommonViews::IconTitle(ICON_MD_CLOUD);
      }
    }
    
    ImGui::EndChild();
  } else {
    // Simple button for items without texture
    std::string noTextureChild = formatString("##tile_%s_%s_no_texture", tile->id.c_str(), tileBrowserId.c_str());
    ImGui::BeginChild(noTextureChild.c_str(), tileSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration);
    if (tile->name.size() > 10) {
      ImGui::PushFont(FontService::getService()->current->sm);
    }
    ImGui::PushID(tile->id.c_str());
    if (ImGui::Button(idAppendedToString(tile->name.c_str(), tile->id).c_str(), tileSize)) {
      if (tileClickCallback != NULL) {
        tileClickCallback(tile);
      }
    }
    ImGui::PopID();

    if (tile->name.size() > 10) {
      ImGui::PopFont();
    }
    tile->dragCallback(tile->id);
    ImGui::EndChild();
  }
  // Handle context menu (right-click) -----------------------------------------------------
  if (ImGui::BeginPopupContextItem(childId.c_str(), ImGuiMouseButton_Right)) {
    for (const auto &item : contextMenuItems) {
      if (ImGui::MenuItem(item.title.c_str())) {
        if (item.action) {
          item.action(tile);
        }
      }
    }
    ImGui::EndPopup();
  }
  // Handle tile layout - this is now managed by the draw() method
  if (ImGui::GetCursorPosX() + tileSize.x + ImGui::GetStyle().ItemSpacing.x < ImGui::GetWindowContentRegionMax().x) {
    ImGui::SameLine();
  }
}

void TileBrowserView::update() {}
