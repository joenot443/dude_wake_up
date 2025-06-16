//
//  HelpService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/17/24.
//

#include "HelpService.hpp"
#include "NodeLayoutView.hpp"
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

bool HelpService::editedBlendModeParameter() {
  if (hasEditedBlendMode) return true;
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeBlend) {
      std::shared_ptr<BlendShader> blendShader = std::dynamic_pointer_cast<BlendShader>(shader);
      
      // Check if Mode have been changed
      hasEditedBlendMode = blendShader->settings->mode->intValue != (int) blendShader->settings->mode->defaultValue;
      return hasEditedBlendMode;
    }
  }
  return false;
}

bool HelpService::editedAlphaParameter() {
  if (hasEditedAlpha) return true;
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeBlend) {
      std::shared_ptr<BlendShader> blendShader = std::dynamic_pointer_cast<BlendShader>(shader);
      
      // Check if Alpha have been changed
      hasEditedAlpha = abs(blendShader->settings->alpha->value - blendShader->settings->alpha->defaultValue) > 0.01;
      return hasEditedAlpha;
    }
  }
  return false;
}

bool HelpService::deletedFirstBlendNode() {
  return NodeLayoutView::getInstance()->hasDeletedBlend;
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
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("start_guide"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("source_drag_instruction"), false);
  CommonViews::xsSpacing();
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
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_effect_title"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale) {
  // Position to right and 2/3 down node
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + (sourceNodeSize.x + 20.0) / scale,
                             sourceNodePosition.y + (sourceNodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  drawLeftArrowView();
  ImGui::SetCursorPosX(x);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30.0);
  CommonViews::H2Title(StringManager::get("connect_source_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("connect_instruction"), false);
  ImGui::SetCursorPosX(x);
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + sourceNodeSize.x / scale + 10.0, sourceNodePosition.y + (sourceNodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
}

void HelpService::drawConnection2View(ImVec2 destNodePosition, ImVec2 destNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(destNodePosition.x - ArrowWidth / scale - 10.0, destNodePosition.y + (destNodeSize.y * (1./3.) * scale) - ArrowHeight / 2.0 / scale));
  float x = ImGui::GetCursorPosX();
  drawRightArrowView();
  x = x - 120.0 / scale;
  ImGui::SetCursorPosX(x);
  CommonViews::H2Title(StringManager::get("complete_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_to_effect_instruction"), false);
}

void HelpService::drawRightArrowView() {
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sin(ofGetElapsedTimef()*5.0)*5.0 + 2.0);
  CommonViews::ImageNamedNew("right-arrow.png", ArrowWidth, ArrowHeight);
}

void HelpService::drawLeftArrowView() {
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + sin(ofGetElapsedTimef()*5.0)*5.0 + 2.0);
  CommonViews::ImageNamedNew("left-arrow.png", ArrowWidth, ArrowHeight);
}

void HelpService::drawUpArrowView() {
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sin(ofGetElapsedTimef()*5.0)*5.0 + 2.0);
  CommonViews::ImageNamedNew("up-arrow.png", ArrowHeight, ArrowHeight);
}

void HelpService::drawSecondSourceView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_second_source_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("add_second_source_instruction"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("webcam_video_instruction"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawSecondSource2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_second_source_title"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawBlendEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("add_blend_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drag_blend_effect_instruction"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawBlendEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("drop_blend_effect_title"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x + LayoutStateService::getService()->nodeLayoutSize().x * 2. / 3.0);
  drawRightArrowView();
}

void HelpService::drawWrongEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("wrong_effect_title"), false);
}

void HelpService::drawBlendConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + (sourceNodeSize.x + 20.0) / scale,
                             sourceNodePosition.y + (sourceNodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  drawLeftArrowView();
  ImGui::SetCursorPosX(x);
  CommonViews::H2Title(StringManager::get("connect_blend_effect_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("combine_nodes_instruction"), false);
  CommonViews::xsSpacing();
}

void HelpService::drawBlendConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  float x = ImGui::GetCursorPosX();
  drawRightArrowView();
  x = x - 400.0 / scale;
  ImGui::SetCursorPosX(x);
  CommonViews::H2Title(StringManager::get("complete_blend_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("connect_to_blend_instruction"), false);
}

void HelpService::drawSecondBlendConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + (sourceNodeSize.x + 20.0) / scale,
                             sourceNodePosition.y + (sourceNodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  drawLeftArrowView();
  ImGui::SetCursorPosX(x);
  CommonViews::H2Title(StringManager::get("add_second_blend_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("second_connection_instruction"), false);
}

void HelpService::drawSecondBlendConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  
  ImGui::SetCursorPos(ImVec2(nodePosition.x + 110.0/scale,
                             nodePosition.y + (nodeSize.y) * (7.0/10.0) / scale));
  drawLeftArrowView();
  ImGui::SetCursorPos(ImVec2(nodePosition.x - 240.0,
                             nodePosition.y + (nodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  ImGui::SetCursorPosX(x);
  CommonViews::H2Title(StringManager::get("complete_second_connection_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("drop_connection_instruction"), false);

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
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  if (!editedBlendModeParameter())  {
    CommonViews::H3Title(StringManager::get("blend_mode_instruction"), false);
    return;
  } else if (!editedAlphaParameter()) {
    CommonViews::H3Title(StringManager::get("alpha_mode_instruction"), false);
    CommonViews::xsSpacing();
    ImGui::SetCursorPosX(x);
    CommonViews::H3Title(StringManager::get("other_nodes_instruction"), false);
    return;
  }
}

void HelpService::drawDeleteBlendShader() {
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * (4.0 / 5.0) - 300.0, ImGui::GetWindowHeight() / 3.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title(StringManager::get("change_parameters_title"), false);
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("delete_blend"), false);
}

void HelpService::drawBlendNodeShortcut(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x + (nodeSize.x + 20.0) / scale,
                             nodePosition.y + (nodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  drawLeftArrowView();
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("blend_shortcut_1"), false);
}


void HelpService::drawBlendNodeShortcut2(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x + (nodeSize.x + 20.0) / scale,
                             nodePosition.y + (nodeSize.y) * (7.0/10.0) / scale));
  float x = ImGui::GetCursorPosX();
  drawLeftArrowView();
  CommonViews::xsSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title(StringManager::get("blend_shortcut_2"), false);
}

void HelpService::drawActionButtons() {
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? LayoutStateService::getService()->browserSize().x : 0;
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - shaderInfoPaneWidth - 400.0, ImGui::GetWindowHeight() - 400.0));
  float x = ImGui::GetCursorPosX();
  ImVec2 iconSize = ImVec2(20.0, 20.0);
  CommonViews::H2Title(StringManager::get("action_buttons_title"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("undo.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("undo_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("redo.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("redo_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("camera.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("camera_view_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("help-outline.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("help_toggle_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("delete.png", iconSize); 
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("clear_canvas_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("focus.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("scale_to_fit_instruction"), false);
  ImGui::SetCursorPosX(x);
  CommonViews::ImageNamedNew("dice.png", iconSize);
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("scale_to_fit_instruction"), false);
  ImGui::SetCursorPosX(x);
  ImGui::PushFont(FontService::getService()->current->p);
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 180.0);
  ImGui::BeginChild("##completeHelpChild", ImVec2(50.0, 50.0), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
  if (ImGui::Button("Okay!")) {
    showPopup = true;
  }
  ImGui::EndChild();
  ImGui::PopFont();
}

void HelpService::drawStageModeHelp() {
  ImVec2 helpSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.0 + 10.0, ImGui::GetContentRegionAvail().y);
  ImGui::BeginChild("stage_mode_help", helpSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
  CommonViews::H2Title(StringManager::get("stage_mode_title"));
  CommonViews::xsSpacing();
  CommonViews::H3Title(StringManager::get("stage_mode_subtitle"));
  CommonViews::xsSpacing();
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
  CommonViews::xsSpacing();
  CommonViews::LargeIconButton(ICON_MD_FAVORITE, "favoriteExample");
  ImGui::SameLine();
  CommonViews::H3Title(StringManager::get("stage_mode_action_favorite"), false);
  ImGui::EndChild();
}

void HelpService::drawCompletionPopup() {
  ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowSize().x / 2.0, 0.0));
  if (ImGui::BeginPopupModal("CompletionPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar)) {
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x / 2.0);
    CommonViews::H2Title(StringManager::get("completion_title"), false);
    CommonViews::xsSpacing();
    CommonViews::H3Title(StringManager::get("completion_description"), false);
    CommonViews::xsSpacing();
    CommonViews::H3Title(StringManager::get("completion_description_2"), false);
    CommonViews::xsSpacing();
    CommonViews::H3Title(StringManager::get("completion_description_3"), false);
    CommonViews::xsSpacing();
    CommonViews::H3BTitle(StringManager::get("completion_description_4"), false);
    
    ImGui::PushFont(FontService::getService()->current->h2);
    ImGui::SetNextItemWidth(60.0);
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2.0 - 30.0);
    if (ImGui::Button("Let me in!")) {
      showPopup = false;
      completed = true;
    }
    ImGui::PopFont();
    ImGui::EndPopup();
  }
  ImGui::OpenPopup("CompletionPopup");
}
