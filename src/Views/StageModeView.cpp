//
//  StageModeView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#include "FontService.hpp"
#include "StageModeView.hpp"
#include "ParameterService.hpp"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "StageParameterView.hpp"
#include "CommonViews.hpp"
#include "Fonts.hpp"

static const float ImGuiWindowTitleBarHeight = 70.0f;

void StageModeView::setup() {
	
}

void StageModeView::update() {
  favoriteParams = ParameterService::getService()->favoriteParameters();
}

void StageModeView::draw() {
  auto stageSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - LayoutStateService::getService()->audioSettingsViewHeight() - ImGuiWindowTitleBarHeight);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0, 10.0));
  ImGui::BeginChild("stage_mode_child", stageSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding);
  ImGui::Columns(2, "stage_mode_view", false);
  
  // FAVORITES 50% | OUTPUT 50%
  ImGui::SetColumnWidth(0, stageSize.x / 2.);
  ImGui::SetColumnWidth(1, stageSize.x / 2.);
  
  // FAVORITES
  auto pos = getScaledCursorScreenLocation();
  ofSetColor(200, 200, 200);
  ofDrawRectangle(pos.x, pos.y, scaleFloat(stageSize.x / 2.) - 20.0, scaleFloat(stageSize.y) - 20.0);
  
  float itemWidth = StageParameterViewSize.x;
  int itemsPerRow = (int)((stageSize.x / 2.) / itemWidth);
  if (itemsPerRow < 1)
    itemsPerRow = 1; // Ensure there is at least one item per row
  
  for (int idx = 0; idx < favoriteParams.size(); ++idx)
  {
    // Draw item
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colorScaledBy(colorFromInt(idx), 0.7).Value);
    StageParameterView::draw(favoriteParams[idx]);
    ImGui::PopStyleColor();
    // Check if we need to wrap to the next line
    if ((idx + 1) % itemsPerRow != 0 && (idx + 1) < favoriteParams.size())
      ImGui::SameLine();
  }
  
  if (favoriteParams.empty()) {
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(5, 5));
    ImGui::PushFont(FontService::getService()->h3);
    ImGui::Text("Parameters you select with a heart appear here.");
    ImGui::PopFont();
  }
  
  ImGui::Columns(2, "stage_mode_view", false);
  ImGui::NextColumn();
  
  // OUTPUT
  std::vector<std::string> shaderIdsToRemove = {};
  for (auto shaderId : ParameterService::getService()->stageShaderIds) {
    auto shader = ShaderChainerService::getService()->shaderForId(shaderId);
    if (shader == nullptr) { shaderIdsToRemove.push_back(shaderId); continue; }
    drawOutput(shader);
  }
  for (auto shaderId : shaderIdsToRemove) {
    ParameterService::getService()->removeStageShaderId(shaderId);
  }
  
  if (ParameterService::getService()->stageShaderIds.size() == 0) {
    auto shader = ShaderChainerService::getService()->shaderForId(ParameterService::getService()->defaultStageShaderIdDepth.first);
    if (shader == nullptr) {
      ParameterService::getService()->defaultStageShaderIdDepth = std::pair<std::string, int>("", -1);
    } else {
      drawOutput(shader);
    }
  }
  
  drawActionButtons();
  ImGui::EndChild();
  ImGui::PopStyleVar();
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
  ImGui::SetCursorScreenPos(ImVec2(getScaledWindowWidth() - 82.0, getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 100.0));
  
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
