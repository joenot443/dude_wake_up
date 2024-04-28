//
//  StageModeView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#include "StageModeView.hpp"
#include "ParameterService.hpp"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "StageParameterView.hpp"
#include "CommonViews.hpp"
#include "Fonts.hpp"


void StageModeView::setup() {
  
}

void StageModeView::update() {
  favoriteParams = ParameterService::getService()->favoriteParameters();
}

void StageModeView::draw() {
  ImGui::BeginChild("stage_mode_child");
  CommonViews::mSpacing();
  
  // |     3/4    |  1/4  |
  float width = ImGui::GetWindowContentRegionMax().x;
  ImGui::Columns(2, "stage_mode_view", false);
  ImGui::SetColumnWidth(0, width / 2.);
  ImGui::SetColumnWidth(1, width / 2.);
  
  auto pos = getScaledCursorScreenLocation();
  auto size = getScaledContentRegionAvail();
  ofSetColor(200, 200, 200);
  ofDrawRectangle(pos.x, pos.y, size.x, size.y);
  
  
  float itemWidth = StageParameterViewSize.x;
  int itemsPerRow = (int)(ImGui::GetContentRegionAvail().x / itemWidth);
  if (itemsPerRow < 1)
    itemsPerRow = 1; // Ensure there is at least one item per row
  std::vector<ImColor> colors = {
     ImColor(73, 36, 62),
    ImColor(112, 66, 100),
    ImColor(187, 132, 147),
    ImColor(219, 175, 160)
  };
  for (int idx = 0; idx < favoriteParams.size(); ++idx)
  {
    // Draw item
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colorFromName(favoriteParams[idx]->settingsName).Value);
    StageParameterView::draw(favoriteParams[idx]);
    ImGui::PopStyleColor();
    // Check if we need to wrap to the next line
    if ((idx + 1) % itemsPerRow != 0 && (idx + 1) < favoriteParams.size())
      ImGui::SameLine();
  }
  
  ImGui::Columns(2, "stage_mode_view", false);
  ImGui::NextColumn();
  
  CommonViews::H3Title("Output");
  for (auto shaderId : ParameterService::getService()->stageShaderIds) {
    auto shader = ShaderChainerService::getService()->shaderForId(shaderId);
    if (shader == nullptr) { ParameterService::getService()->removeStageShaderId(shaderId); continue; }
    drawOutput(shader);
  }
  
  if (ParameterService::getService()->stageShaderIds.size() == 0) {
    CommonViews::H3Title("Star a Node to view it in Stage Mode!");
  }
  
  drawActionButtons();
  ImGui::EndChild();
}

void StageModeView::drawOutput(std::shared_ptr<Shader> output) {

  float width = ImGui::GetContentRegionAvail().x;
  ImTextureID texID = (ImTextureID)(uintptr_t)output->frame()->getTexture().getTextureData().textureID;
  ImGui::Image(texID, ImVec2(width, (9. / 16.) * width));
}

void StageModeView::drawActionButtons()
{
  auto pos = ImGui::GetCursorScreenPos();
  
  // Set the cursor to the bottom right of the window
  ImGui::SetCursorScreenPos(ImVec2(getScaledWindowWidth() - 150.0, getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 100.0));
  
  // Draw the node mode button
  if (CommonViews::LargeIconButton(ICON_MD_NETWORK_CELL, "stageModeDisable"))
  {
    LayoutStateService::getService()->stageModeEnabled = false;
  }
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  ImGui::SameLine();
  
  ImGui::SetCursorPos(pos);
}
