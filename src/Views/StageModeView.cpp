//
//  StageModeView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/11/24.
//

#include "FontService.hpp"
#include "HelpService.hpp"
#include "StageModeView.hpp"
#include "ParameterService.hpp"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "StageParameterView.hpp"
#include "CommonViews.hpp"
#include "Fonts.hpp"

static const float ImGuiWindowTitleBarHeight = 30.0f;

void StageModeView::setup() {
	
}

void StageModeView::update() {
  favoriteParams = ParameterService::getService()->favoriteParameters();
  allParams = ParameterService::getService()->allActiveParameters();
  // Filter to exclude Parameters of type ParameterType_Hidden
  allParams.erase(std::remove_if(allParams.begin(), allParams.end(), [](std::shared_ptr<Parameter> p) { return p->type == ParameterType_Hidden; }), allParams.end());
}

void StageModeView::draw() {
  ImVec2 stageSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - LayoutStateService::getService()->audioSettingsViewHeight() - ImGuiWindowTitleBarHeight);
  HelpService::getService()->hasOpenedStageMode = true;
  
  float topY = ImGui::GetCursorPosY();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0, 10.0));
  ImGui::BeginChild("stage_mode_child", stageSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
//  ImGui::Columns(2, "stage_mode_view", false);
  
  float stageParamHeight = LayoutStateService::getService()->helpEnabled ? stageSize.y / 2.0 : stageSize.y;
  ImGui::BeginChild("stage_mode_parameters", ImVec2(stageSize.x / 2., stageParamHeight), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
  
  // FAVORITES 50% | OUTPUT 50%
//  ImGui::SetColumnWidth(0, stageSize.x / 2.);
//  ImGui::SetColumnWidth(1, stageSize.x / 2.);
  
  // FAVORITES
  ImVec2 pos = getScaledCursorScreenLocation();
//  ofSetColor(200, 200, 200);
//  ofDrawRectangle(pos.x, pos.y, scaleFloat(stageSize.x / 2.) - 20.0, scaleFloat(stageParamHeight) - 20.0);
  
  float itemWidth = StageParameterViewSize.x;
  int itemsPerRow = (int)(((stageSize.x / 2.) - 20.0) / itemWidth) - 1;
  if (itemsPerRow < 1)
    itemsPerRow = 1; // Ensure there is at least one item per row
  
  if (LayoutStateService::getService()->allParametersInStageModeEnabled) {
    drawAllParams(itemsPerRow);
  } else {
    drawFavoriteParams(itemsPerRow);
  }
  ImGui::EndChild();
  
  // Draw Help Quadrant
  if (LayoutStateService::getService()->helpEnabled) {
    drawHelp();
  }
  
  ImGui::SetCursorPos(ImVec2(stageSize.x / 2. + 20.0, topY));
  
  ImGui::BeginChild("stage_mode_output", ImVec2(stageSize.x / 2., stageSize.y - 40.0), true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
  ImGui::NewLine();
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
  
  // If we haven't specified output Shaders for StageMode but we DO have shaders active
  if (ParameterService::getService()->stageShaderIds.size() == 0 &&
      ShaderChainerService::getService()->shaders().size() != 0) {
    auto shader = ShaderChainerService::getService()->shaderForId(ParameterService::getService()->defaultStageShaderIdDepth.first);
    if (shader == nullptr) {
      ParameterService::getService()->defaultStageShaderIdDepth = std::pair<std::string, int>("", -1);
    } else {
      drawOutput(shader);
    }
  }
  
  if (LayoutStateService::getService()->helpEnabled) {
    HelpService::getService()->drawStageModeActionButtonsHelp();
  }
  
  drawActionButtons();
  ImGui::EndChild();
  ImGui::EndChild();
  ImGui::PopStyleVar();
}

void StageModeView::drawAllParams(int itemsPerRow) {
  int colorIndex = -1;
  std::string lastSettingsId = "";
  int rowCount = 0;
  std::set<std::shared_ptr<Parameter>> params = std::set<std::shared_ptr<Parameter>>(allParams.begin(), allParams.end());
  for (auto param : params)
  {
    if (rowCount <= itemsPerRow) {
      ImGui::SameLine();
    } else {
      CommonViews::HSpacing(1);
      rowCount = 0;
    }
    
    if (lastSettingsId != param->ownerSettingsId) {
      lastSettingsId = param->ownerSettingsId;
      colorIndex++;
    }
    
    // Draw item
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colorScaledBy(colorFromInt(colorIndex), 0.7).Value);
    StageParameterView::draw(param);
    ImGui::PopStyleColor();
    
    rowCount += 1;
  }
  
  if (allParams.empty()) {
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(5, 5));
    ImGui::PushFont(FontService::getService()->h3);
    ImGui::Text("Parameters for all effects will appear here.");
    ImGui::PopFont();
  }
}

void StageModeView::drawFavoriteParams(int itemsPerRow) {
  for (int idx = 0; idx < favoriteParams.size(); ++idx)
  {
    // Draw item
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colorScaledBy(colorFromInt(idx), 0.7).Value);
    StageParameterView::draw(favoriteParams[idx]->parameter);
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
  if (CommonViews::LargeIconButton(ICON_MD_HUB, "stageModeDisable"))
  {
    LayoutStateService::getService()->stageModeEnabled = false;
  }
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  ImGui::SameLine();
  
  // Draw the Favorite/All button
  auto icon = LayoutStateService::getService()->allParametersInStageModeEnabled ? ICON_MD_FAVORITE : ICON_MD_FAVORITE_OUTLINE;
  if (CommonViews::LargeIconButton(icon, "stageModeDisable"))
  {
    LayoutStateService::getService()->allParametersInStageModeEnabled = !LayoutStateService::getService()->allParametersInStageModeEnabled;
  }
  
  ImGui::SetCursorPos(pos);
}

void StageModeView::drawHelp()
{
  HelpService::getService()->drawStageModeHelp();
}
