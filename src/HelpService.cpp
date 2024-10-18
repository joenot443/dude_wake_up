//
//  HelpService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/17/24.
//

#include "HelpService.hpp"
#include "Fonts.hpp"
#include "FontService.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "BlendShader.hpp"
#include "LayoutStateService.hpp"
#include "CommonViews.hpp"
#include "imgui.h"

static float ArrowHeight = 40.0;
static float ArrowWidth = 60.0;

bool HelpService::placedSource() {
  return VideoSourceService::getService()->videoSources().size() > 0;
}

bool HelpService::placedEffect() {
  return ShaderChainerService::getService()->shaders().size() > 0;
}

bool HelpService::madeConnection() {
  return ShaderChainerService::getService()->connections().size() > 0;
}

bool HelpService::placedSecondSource() {
  return VideoSourceService::getService()->videoSources().size() > 1;
}

bool HelpService::placedBlendEffect() {
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeBlend) return true;
  }
  return false;
}

bool HelpService::madeFirstBlendConnection() {
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeBlend)) return true;
  }
  return false;
}

bool HelpService::madeSecondBlendConnection() {
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeBlend)) {
      return connection->end->inputs.size() > 1;
    }
  }
  return false;
}

bool HelpService::openedShaderInfo() {
  return LayoutStateService::getService()->shouldDrawShaderInfo();
}

bool HelpService::editedBlendParameter() {
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeBlend) {
      std::shared_ptr<BlendShader> blendShader = std::dynamic_pointer_cast<BlendShader>(shader);
      
      // Check if Mode and Alpha have been changed
      return blendShader->settings->mode->intValue != (int) blendShader->settings->mode->defaultValue  &&
      abs(blendShader->settings->alpha->value - blendShader->settings->alpha->defaultValue) > 0.01;
    }
  }
  return false;
}

bool HelpService::openedStageMode() {
  bool stageMode = LayoutStateService::getService()->stageModeEnabled;
  if (stageMode) {
    hasOpenedStageMode = true;
  }
  return stageMode;
}

void HelpService::drawSourceView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("welcome_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("welcome_subtitle"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("start_guide"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("source_drag_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawSource2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H3Title(StringManager::get("drop_source_instruction"), false);
}

void HelpService::drawEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("effect_drag_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_effect_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x, sourceNodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("connect_source_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("connect_instruction"), false);
  ImGui::SetCursorPosX(x);
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + sourceNodeSize.x / scale + 10.0, sourceNodePosition.y + (sourceNodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawConnection2View(ImVec2 destNodePosition, ImVec2 destNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(destNodePosition.x, destNodePosition.y - 50.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("complete_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_to_effect_instruction"), false);
  ImGui::SetCursorPos(ImVec2(destNodePosition.x - ArrowWidth - 10.0, destNodePosition.y + (destNodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawRightArrowView() {
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sin(ofGetElapsedTimef()*5.0)*5.0 + 5.0);
  CommonViews::ImageNamed("right-arrow.png", ArrowWidth, ArrowHeight);
}

void HelpService::drawLeftArrowView() {
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sin(ofGetElapsedTimef()*5.0)*5.0 + 5.0);
  CommonViews::ImageNamed("left-arrow.png", ArrowWidth, ArrowHeight);
}

void HelpService::drawSecondSourceView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_second_source_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("add_second_source_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("webcam_video_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawSecondSource2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_second_source_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawBlendEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_blend_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_blend_effect_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawBlendEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_blend_effect_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + ImGui::GetWindowSize().x / 3.0);
  drawRightArrowView();
}

void HelpService::drawWrongEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("wrong_effect_title"), false);
}

void HelpService::drawBlendConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + 400 / scale));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("connect_blend_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("combine_nodes_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawBlendConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + 400 / scale));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("complete_blend_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("connect_to_blend_instruction"), false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawSecondBlendConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + 400 / scale));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_second_blend_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("second_connection_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawSecondBlendConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("complete_second_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drop_connection_instruction"), false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0,
                             nodePosition.y + (nodeSize.y / scale / 2.0) + (10.0 / scale) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawOpenShaderInfoView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("open_shader_info_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("click_blend_instruction"), false);
}

void HelpService::drawEditParametersShaderInfoPane() {
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * (4.0 / 5.0) - 300.0, ImGui::GetWindowHeight() / 3.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("change_parameters_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_slider_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("effects_settings_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("other_nodes_instruction"), false);
}

void HelpService::drawActionButtons() {
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? LayoutStateService::getService()->browserSize().x : 0;
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - shaderInfoPaneWidth - 400.0, ImGui::GetWindowHeight() - 400.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("action_buttons_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_HELP_OUTLINE, "helpExample"); ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("help_toggle_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_DELETE_FOREVER, "resetExample"); ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("clear_canvas_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_CENTER_FOCUS_WEAK, "navigateExample"); ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("camera_view_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_VIEW_AGENDA, "stageModeExample"); ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("stage_mode_instruction"), false); ImGui::SameLine();
  CommonViews::H3BTitle(StringManager::get("press_now_instruction"), false);
}

void HelpService::drawStageModeHelp() {
  ImVec2 helpSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.0 + 10.0, ImGui::GetContentRegionAvail().y);
  ImGui::BeginChild("stage_mode_help", helpSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
  CommonViews::H2Title(StringManager::get("stage_mode_title"));
  CommonViews::mSpacing();
  CommonViews::H3Title(StringManager::get("stage_mode_subtitle"));
  CommonViews::mSpacing();
  CommonViews::H3Title(StringManager::get("stage_mode_description"));
  ImGui::EndChild();
}

void HelpService::drawStageModeActionButtonsHelp() {
  ImVec2 helpSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.0, ImGui::GetContentRegionAvail().y / 2.0);
  CommonViews::lSpacing();
  ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2.0);
  ImGui::BeginChild("stage_mode_actions", helpSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
  
  CommonViews::LargeIconButton(ICON_MD_HUB, "nodeModeExample");
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("stage_mode_action_toggle"), false);
  CommonViews::mSpacing();
  CommonViews::LargeIconButton(ICON_MD_FAVORITE, "favoriteExample");
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("stage_mode_action_favorite"), false);
  ImGui::EndChild();
}

void HelpService::drawCompletionPopup() {
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x / 4.0, ImGui::GetWindowSize().y / 3.0));
  if (ImGui::BeginPopupModal("CompletionPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {
    CommonViews::H2Title(StringManager::get("completion_title"), false);
    CommonViews::mSpacing();
    CommonViews::H3Title(StringManager::get("completion_description"), false);
    CommonViews::mSpacing();
    CommonViews::H3Title(StringManager::get("completion_description_2"), false);
    CommonViews::mSpacing();
    CommonViews::H3Title(StringManager::get("completion_description_3"), false);
    CommonViews::mSpacing();
    CommonViews::H3BTitle(StringManager::get("completion_description_4"), false);
    
    ImGui::PushFont(FontService::getService()->h2);
    ImGui::SetNextItemWidth(60.0);
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2.0 - 30.0);
    if (ImGui::Button("Okay!")) {
      completed = true;
    }
    ImGui::PopFont();
    ImGui::EndPopup();
  }
  ImGui::OpenPopup("CompletionPopup");
}
