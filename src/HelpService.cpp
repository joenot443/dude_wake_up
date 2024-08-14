//
//  HelpService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/17/24.
//

#include "HelpService.hpp"
#include "Fonts.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "MixShader.hpp"
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

bool HelpService::placedMixEffect() {
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeMix) return true;
  }
  return false;
}

bool HelpService::madeFirstMixConnection() {
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeMix)) return true;
  }
  return false;
}

bool HelpService::madeSecondMixConnection() {
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeMix)) {
      return connection->end->inputs.size() > 1;
    }
  }
}

bool HelpService::openedShaderInfo() {
  return LayoutStateService::getService()->shouldDrawShaderInfo();
}

bool HelpService::editedMixParameter() {
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeMix) {
      std::shared_ptr<MixShader> mixShader = std::dynamic_pointer_cast<MixShader>(shader);
      return abs(mixShader->settings->mix->value - mixShader->settings->mix->defaultValue) > 0.1;
    }
  }
}

bool HelpService::openedStageMode() {
  bool stageMode = LayoutStateService::getService()->stageModeEnabled;
  if (stageMode) {
    LayoutStateService::getService()->helpEnabled = false;
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

void HelpService::drawMixEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_mix_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_mix_effect_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawMixEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_mix_effect_title"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + ImGui::GetWindowSize().x / 3.0);
  drawRightArrowView();
}

void HelpService::drawWrongEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("wrong_effect_title"), false);
}

void HelpService::drawMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("connect_mix_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("combine_nodes_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 50.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("complete_mix_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("connect_to_mix_instruction"), false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawSecondMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_second_mix_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("second_connection_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawSecondMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("complete_second_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drop_connection_instruction"), false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) + (10.0 / scale) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawOpenShaderInfoView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("open_shader_info_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("click_mix_instruction"), false);
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
  CommonViews::LargeIconButton(ICON_MD_HELP_OUTLINE, "helpExample"); ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("help_toggle_instruction"), false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_RECYCLING, "resetExample"); ImGui::SameLine();
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
