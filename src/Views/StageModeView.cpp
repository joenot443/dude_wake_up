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
#include "VideoSourceService.hpp"
#include "ActionService.hpp"
#include "Colors.hpp"
#include "OscillationService.hpp"

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
  bool hasOutput = false;
  
  for (auto shaderId : ParameterService::getService()->stageShaderIds) {
    auto shader = ShaderChainerService::getService()->shaderForId(shaderId);
    if (shader == nullptr) { shaderIdsToRemove.push_back(shaderId); continue; }
    drawOutputShader(shader);
    hasOutput = true;
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
      drawOutputShader(shader);
      hasOutput = true;
    }
  }
  
  // NEW: If no shaders available, fall back to video sources
  if (!hasOutput) {
    auto videoSources = VideoSourceService::getService()->videoSources();
    if (!videoSources.empty()) {
      // Use the last video source in the array as requested
      auto videoSource = videoSources.back();
      drawOutputVideoSource(videoSource);
      hasOutput = true;
    }
  }
  
  if (LayoutStateService::getService()->helpEnabled) {
    HelpService::getService()->drawStageModeActionButtonsHelp();
  }
  
  // NEW: Draw the action bar instead of the old action buttons
  drawActionBar();
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
    ImGui::PushFont(FontService::getService()->current->h3);
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
    ImGui::PushFont(FontService::getService()->current->h3);
    ImGui::Text("Parameters you select with a heart appear here.");
    ImGui::PopFont();
  }
  
}

void StageModeView::drawOutputShader(std::shared_ptr<Shader> output) {
  float width = ImGui::GetContentRegionAvail().x;
  ImTextureID texID = (ImTextureID)(uintptr_t)output->frame()->getTexture().getTextureData().textureID;
  ImGui::Image(texID, ImVec2(width, (9. / 16.) * width));
}

void StageModeView::drawOutputVideoSource(std::shared_ptr<VideoSource> output) {
  float width = ImGui::GetContentRegionAvail().x;
  ImTextureID texID = (ImTextureID)(uintptr_t)output->frame()->getTexture().getTextureData().textureID;
  ImGui::Image(texID, ImVec2(width, (9. / 16.) * width));
}

void StageModeView::drawActionBar()
{
  ImVec2 pos = ImGui::GetCursorPos();
  int buttonCount = 7;
  float buttonWidth = 30.0;
  ImVec2 imageSize = ImVec2(buttonWidth, buttonWidth);
  ImVec2 imageRatio = ImVec2(1.5, 1.5);
  
  // Position for either the collapsed or expanded action bar
  float yPos = getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 200.0;
  
  if (!LayoutStateService::getService()->actionBarExpanded) {
    // Draw collapsed state with single button
    ImVec2 collapsedPos = ImVec2(getScaledWindowWidth() - 42, yPos);
    ImGui::SetCursorPos(collapsedPos);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::ActionBarBackgroundColor.Value);
    ImGui::BeginChild("##CollapsedActionBar", ImVec2(buttonWidth, buttonWidth), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
    
    if (CommonViews::ImageButton("expand", "back.png", imageSize, imageRatio, true)) {
      LayoutStateService::getService()->actionBarExpanded = true;
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    ImGui::SetCursorPos(pos);
    return;
  }
  
  // Draw expanded action bar
  float actionBarWidth = buttonWidth * buttonCount * imageRatio.x + 30.0;
  ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x - actionBarWidth - 12.0, yPos));
  
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::ActionBarBackgroundColor.Value);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
  ImGui::SetNextItemAllowOverlap();
  
  
  ImGui::BeginChild("##ActionButtons", ImVec2(actionBarWidth, buttonWidth), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
  // Collapse button
  if (CommonViews::ImageButton("collapse", "forward.png", imageSize, imageRatio, true)) {
    LayoutStateService::getService()->actionBarExpanded = false;
  }
  ImGui::SameLine(0, 10);
  
  // Draw the undo button
  if (CommonViews::ImageButton("undo", "undo.png", imageSize, imageRatio, true)) {
    if (ActionService::getService()->canUndo()) {
      ActionService::getService()->undo();
    }
  }
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("Undo the last action (Cmd+Z)");
  ImGui::SameLine(0, 10);
  
  // Draw the redo button
  if (CommonViews::ImageButton("redo", "redo.png", imageSize, imageRatio, true)) {
    if (ActionService::getService()->canRedo()) {
      ActionService::getService()->redo();
    }
  }
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("Redo the last undone action (Cmd+Y)");
  ImGui::SameLine(0, 10);
  
  // Draw the capture screenshot button
  if (CommonViews::ImageButton("capture", "camera.png", imageSize, imageRatio, true)) {
    VideoSourceService::getService()->captureOutputWindowScreenshot();
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Capture a screenshot of the output window");
  ImGui::SameLine(0, 10);
  
  // Draw the help button
  std::string helpImage = LayoutStateService::getService()->helpEnabled ? "help-outline.png" : "help.png";
  if (CommonViews::ImageButton("help", helpImage, imageSize, imageRatio, true)) {
    LayoutStateService::getService()->helpEnabled = !LayoutStateService::getService()->helpEnabled;
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Toggle help overlay");
  ImGui::SameLine(0, 10);
  
  // Draw the reset button
  if (CommonViews::ImageButton("reset", "delete.png", imageSize, imageRatio, true)) {
    clear();
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Remove all nodes and connections (Cmd+N)");
  ImGui::SameLine(0, 10);
  
  // Draw the node layout mode button (instead of stage mode since we're already in stage mode)
  if (CommonViews::ImageButton("nodemode", "nodes.png", imageSize, imageRatio, true)) {
    LayoutStateService::getService()->stageModeEnabled = false;
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Switch to node layout mode (Cmd+B)");
  ImGui::SameLine(0, 10);
  
  // Draw the Favorite/All button (from the old action buttons)
  auto icon = LayoutStateService::getService()->allParametersInStageModeEnabled ? "heart-fill.png" : "heart.png";
  if (CommonViews::ImageButton("favoriteAll", icon, imageSize, imageRatio, true)) {
    LayoutStateService::getService()->allParametersInStageModeEnabled = !LayoutStateService::getService()->allParametersInStageModeEnabled;
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Toggle between favorite and all parameters");
  
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
  ImGui::SetCursorPos(pos);
}

void StageModeView::clear() {
  ShaderChainerService::getService()->clear();
  VideoSourceService::getService()->clear();
  ParameterService::getService()->clear();
  ActionService::getService()->clear();
  OscillationService::getService()->clear();
}

void StageModeView::drawHelp()
{
  HelpService::getService()->drawStageModeHelp();
}
