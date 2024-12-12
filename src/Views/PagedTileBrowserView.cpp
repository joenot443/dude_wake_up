#include "PagedTileBrowserView.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"

static const ImVec2 TileSize = ImVec2(88, 55);

void PagedTileBrowserView::setup() {}

void PagedTileBrowserView::setTileItems(std::vector<std::shared_ptr<TileItem>> items) {
  tileItems = items;
  currentPage = 0;
}

void PagedTileBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  tileClickCallback = callback;
}

void PagedTileBrowserView::draw() {
  int itemsPerPage = getItemsPerPage();
  int startIdx = currentPage * itemsPerPage;
  int endIdx = std::min(startIdx + itemsPerPage, (int)tileItems.size());
  
  // Draw the grid
  for (int i = 0; i < rowCount; i++) {
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
          ImVec2 endPos = ImVec2(startPos.x + TileSize.x, startPos.y + TileSize.y);
          ImGui::GetWindowDrawList()->AddImageRounded(tile->textureID, startPos, endPos, ImVec2(0,0), ImVec2(1,1), ImColor(255, 255, 255), 5.0);
          
          ImGui::GetWindowDrawList()->AddRectFilled(
            startPos, 
            endPos,
            IM_COL32(0, 0, 0, 50)
          );
          
          ImGui::SetCursorPos(startPos);
          
          ImGui::PushFont(FontService::getService()->p);
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 1.0, 1.0, 0.6));
          if (ImGui::Button(tile->name.c_str(), TileSize)) {
            if (tileClickCallback) tileClickCallback(tile);
          }
          ImGui::PopStyleColor();
          ImGui::PopFont();
          
          tile->dragCallback();
        } else {
          if (ImGui::Button(tile->name.c_str(), TileSize)) {
            if (tileClickCallback) tileClickCallback(tile);
          }
          tile->dragCallback();
        }
      } else {
        // Empty tile placeholder
        ImGui::Dummy(TileSize);
      }
      
      // Add horizontal spacing after each tile (except last column)
      if (j < columnCount - 1) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(5.0f, 0.0f));
      }
    }
    
    // Add vertical spacing after each row (except last row)
    if (i < rowCount - 1) {
      ImGui::Dummy(ImVec2(0.0f, 5.0f));
    }
  }
  
  // Navigation buttons
  ImGui::NewLine();
  bool canGoPrev = currentPage > 0;
  bool canGoNext = currentPage < getTotalPages() - 1;
  
  if (canGoPrev) {
    if (ImGui::Button("Previous")) currentPage--;
  } else {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::Button("Previous");
    ImGui::PopStyleVar();
  }
  
  ImGui::SameLine();
  ImGui::Text("Page %d/%d", currentPage + 1, getTotalPages());
  
  ImGui::SameLine();
  if (canGoNext) {
    if (ImGui::Button("Next")) currentPage++;
  } else {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    ImGui::Button("Next");
    ImGui::PopStyleVar();
  }
}

void PagedTileBrowserView::update() {} 
