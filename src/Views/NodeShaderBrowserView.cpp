#include "NodeShaderBrowserView.hpp"
#include "ShaderChainerService.hpp"
#include "CommonViews.hpp"

void NodeShaderBrowserView::setup() {
  // Initialize all browsers with 4x3 grid
  basicTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  filterTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  glitchTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  transformTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  mixTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  maskTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);

  // Set sizes
  for (auto* view : {&basicTileBrowserView, &filterTileBrowserView, &glitchTileBrowserView,
                     &transformTileBrowserView, &mixTileBrowserView, &maskTileBrowserView}) {
    (*view)->size = size;
  }

  // Set tile items
  auto service = ShaderChainerService::getService();
  basicTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableBasicShaders));
  filterTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableFilterShaders));
  glitchTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableGlitchShaders));
  transformTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableTransformShaders));
  mixTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableMixShaders));
  maskTileBrowserView->setTileItems(tileItemsForShaders(service->auxillaryAvailableMaskShaders));
}

void NodeShaderBrowserView::drawTabButtons() {
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
  
  for (int i = 0; i < 6; i++) {
    if (i > 0) ImGui::SameLine();
    
    bool isSelected = (currentTab == i);
    if (isSelected) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }
    
    if (ImGui::Button(tabLabels[i], ImVec2(0, 0))) {
      currentTab = i;
    }
    
    if (isSelected) {
      ImGui::PopStyleColor();
    }
  }
  
  ImGui::PopStyleVar();
}

void NodeShaderBrowserView::drawSelectedBrowser() {
  switch (currentTab) {
    case 0: basicTileBrowserView->draw(); break;
    case 1: filterTileBrowserView->draw(); break;
    case 2: glitchTileBrowserView->draw(); break;
    case 3: transformTileBrowserView->draw(); break;
    case 4: mixTileBrowserView->draw(); break;
    case 5: maskTileBrowserView->draw(); break;
  }
}

void NodeShaderBrowserView::draw() {
  drawTabButtons();
  ImGui::Spacing();
  drawSelectedBrowser();
}

void NodeShaderBrowserView::setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) {
  basicTileBrowserView->setCallback(callback);
  filterTileBrowserView->setCallback(callback);
  glitchTileBrowserView->setCallback(callback);
  transformTileBrowserView->setCallback(callback);
  mixTileBrowserView->setCallback(callback);
  maskTileBrowserView->setCallback(callback);
} 
