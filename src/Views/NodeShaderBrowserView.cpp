#include "NodeShaderBrowserView.hpp"
#include "ShaderChainerService.hpp"
#include "CommonViews.hpp"

void NodeShaderBrowserView::setup() {
  basicTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  filterTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  glitchTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  transformTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  mixTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  maskTileBrowserView = std::make_unique<PagedTileBrowserView>(3, 3);
  
  // Set sizes and padding
  for (auto* view : {&basicTileBrowserView, &filterTileBrowserView, &glitchTileBrowserView,
    &transformTileBrowserView, &mixTileBrowserView, &maskTileBrowserView}) {
      (*view)->setWidth(size.x);
      (*view)->leftPadding = leftPadding;
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

void NodeShaderBrowserView::applyButtonStyles(bool isSelected) {
  if (isSelected) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::ButtonSelected.Value);
  } else {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // transparent
  }
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::ButtonSelectedHovered.Value);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Colors::ButtonSelected.Value);
  ImGui::PushStyleColor(ImGuiCol_Border, Colors::NodeBorderColor.Value);
}

void NodeShaderBrowserView::popButtonStyles(bool isSelected) {
  if (isSelected) {
    ImGui::PopStyleVar(2);
  }
  ImGui::PopStyleColor(4);
}

void NodeShaderBrowserView::drawTabButtons() {
  ImGui::Dummy(ImVec2(leftPadding, 0.0));
  ImGui::SameLine();
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
  
  float buttonWidth = (size.x - leftPadding) / 6.0f;
  for (int i = 0; i < 6; i++) {
    if (i > 0) ImGui::SameLine();
    
    bool isSelected = (currentTab == i);
    applyButtonStyles(isSelected);
    
    if (ImGui::Button(tabLabels[i], ImVec2(buttonWidth, 0))) {
      currentTab = i;
    }
    
    popButtonStyles(isSelected);
  }
  
  ImGui::PopStyleVar(); // Pop ItemSpacing
  
  ImGui::Dummy(ImVec2(0.0, 10.0));
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
