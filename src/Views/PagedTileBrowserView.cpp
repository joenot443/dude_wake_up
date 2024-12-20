#include "PagedTileBrowserView.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"

static const float TILE_ASPECT_RATIO = 88.0f / 55.0f;  // Original ratio
static const float TILE_PADDING = 5.0f;  // Padding between tiles

void PagedTileBrowserView::setup() {}

void PagedTileBrowserView::setTileItems(std::vector<std::shared_ptr<TileItem>> items) {
  tileItems = items;
  currentPage = 0;
}

void PagedTileBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  tileClickCallback = callback;
}

void PagedTileBrowserView::setWidth(float width) {
  viewWidth = width;
}

ImVec2 PagedTileBrowserView::calculateTileSize() const {
  if (viewWidth <= 0.0f) {
    return ImVec2(88, 55);  // Default size if no width set
  }
  
  // Calculate available width considering padding between tiles
  float totalPadding = TILE_PADDING * (columnCount - 1);
  float availableWidth = viewWidth - totalPadding;
  
  // Calculate tile width
  float tileWidth = availableWidth / columnCount;
  // Calculate tile height maintaining aspect ratio
  float tileHeight = tileWidth / TILE_ASPECT_RATIO;
  
  return ImVec2(tileWidth, tileHeight);
}

void PagedTileBrowserView::draw() {
  ImVec2 tileSize = calculateTileSize();
  int itemsPerPage = getItemsPerPage();
  int startIdx = currentPage * itemsPerPage;
  int endIdx = std::min(startIdx + itemsPerPage, (int)tileItems.size());
  
  // Draw the grid
  for (int i = 0; i < rowCount; i++) {
    // Add left padding at the start of each row
    ImGui::Dummy(ImVec2(leftPadding, 0.0f));
    ImGui::SameLine();
    
    for (int j = 0; j < columnCount; j++) {
      int idx = startIdx + i * columnCount + j;
      
      // Add horizontal spacing between tiles (except for first column)
      if (j > 0) {
        ImGui::SameLine();
      }
      
      if (idx < endIdx) {
        auto tile = tileItems[idx];
        ImVec2 startPos = ImGui::GetCursorPos();
        
        if (tile->textureID != nullptr) {
          ImVec2 endPos = ImVec2(startPos.x + tileSize.x, startPos.y + tileSize.y);
          ImGui::GetWindowDrawList()->AddImageRounded(tile->textureID, startPos, endPos, ImVec2(0,0), ImVec2(1,1), ImColor(255, 255, 255), 5.0);

          ImGui::SetCursorPos(startPos);
          
          ImGui::PushFont(FontService::getService()->p);
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.6));
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0, 0.0, 0.0, 0.0));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0, 0.0, 0.0, 0.0));
          if (ImGui::Button(tile->name.c_str(), tileSize)) {
            if (tileClickCallback) tileClickCallback(tile);
          }

          ImColor color = ImGui::IsItemHovered() ? ImColor(255, 255, 255, 100) : ImColor(255, 255, 255, 50);
          // If item is active, make it darker
          if (ImGui::IsItemActive()) {
            color = ImColor(255, 255, 255, 150);
          }
          ImGui::GetWindowDrawList()->AddRectFilled(
            startPos, 
            endPos,
            color,
            5.0
          );
          ImGui::PopStyleColor(4);
          ImGui::PopFont();
          
          tile->dragCallback();
        } else {
          if (ImGui::Button(tile->name.c_str(), tileSize)) {
            if (tileClickCallback) tileClickCallback(tile);
          }
          tile->dragCallback();
        }
      } else {
        // Empty tile placeholder
        ImGui::Dummy(tileSize);
      }
      
      // Add horizontal spacing after each tile (except last column)
      if (j < columnCount - 1) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(TILE_PADDING, 0.0f));
      }
    }
    
    // Add vertical spacing after each row (except last row)
    if (i < rowCount - 1) {
      ImGui::Dummy(ImVec2(0.0f, TILE_PADDING));
    }
  }
  
  // Navigation buttons
  ImGui::NewLine();
  bool canGoPrev = currentPage > 0;
  bool canGoNext = currentPage < getTotalPages() - 1;
  
  if (canGoPrev) {
    if (CommonViews::SmallImageButton("prev", "back.png")) currentPage--;
  } 
  
  ImGui::SameLine();
  ImGui::Text("%d/%d", currentPage + 1, getTotalPages());
  
  ImGui::SameLine();
  if (canGoNext) {
    if (CommonViews::SmallImageButton("next", "forward.png")) currentPage++;
  } 
}

void PagedTileBrowserView::update() {} 
