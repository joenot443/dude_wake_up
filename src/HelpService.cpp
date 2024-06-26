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
  //  if (!(ShaderChainerService::getService()->shaders().size() > 1)) return false;
  
  auto shaders = ShaderChainerService::getService()->shaders();
  for (auto shader : shaders) {
    if (shader->type() == ShaderTypeMix) return true;
  }
  return false;
}

bool HelpService::madeFirstMixConnection() {
  if (!(ShaderChainerService::getService()->connections().size() > 1)) return false;
  
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    // We have a Mix which has at least one input
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeMix)) return true;
  }
  return false;
}

bool HelpService::madeSecondMixConnection() {
  if (!(ShaderChainerService::getService()->connections().size() > 1)) return false;
  
  auto connections = ShaderChainerService::getService()->connections();
  for (auto connection : connections) {
    if (ShaderChainerService::getService()->isShaderType(connection->end, ShaderTypeMix)) {
      // We have more than one Input on our Mix
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
  CommonViews::H2Title("Welcome to nottawa!", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Thank you so much for being here. \n \nTo skip this tutorial, click the ? icon in the bottom right.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Everything in nottawa starts with a Source. A Source can be a live generated scene (\"shader\"), video file, webcam feed, and more.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("To start, click and drag a Source from the browser over to here in the canvas.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawSource2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H3Title("Now drop the Source anywhere you like on the canvas.", false);
}

void HelpService::drawEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Add an Effect", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Grab an Effect and drag it here to the canvas.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Drop the Effect anywhere you like on the canvas.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x, sourceNodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Let's connect our Source to the Effect.", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Click and drag from the > icon on the Source \nto connect it to the o on the left side of the Effect", false);
  ImGui::SetCursorPosX(x);
  ImGui::SetCursorPos(ImVec2(sourceNodePosition.x + sourceNodeSize.x / scale + 10.0, sourceNodePosition.y + (sourceNodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawConnection2View(ImVec2 destNodePosition, ImVec2 destNodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(destNodePosition.x, destNodePosition.y - 50.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Complete the connection", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Drag here to the o icon on your Effect", false);
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
  CommonViews::H2Title("Adding another Source", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Add a second Source to the canvas", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("You can also use a Webcam or video from the Library.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}


void HelpService::drawSecondSource2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Drop the Source on the canvas", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + 200.0);
  drawRightArrowView();
}

void HelpService::drawMixEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Add a Mix Effect", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Now you can grab an Effect and drag it here to the canvas", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  drawLeftArrowView();
}

void HelpService::drawMixEffect2View() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Drop the Mix Effect on the canvas", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x + ImGui::GetWindowSize().x / 3.0);
  drawRightArrowView();
}

void HelpService::drawWrongEffectView() {
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Whoops! That's not a Mix Effect", false);
}

void HelpService::drawMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Connect to the Mix Effect", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Let's combine all the nodes together. Drag from the output on this node to one of the inputs on Mix.", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 50.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Complete the connection", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Connect with one of the two inputs on Mix", false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawSecondMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y - 90 / scale - 100.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Add a second connection to the Mix", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Drag from here to an input on Mix", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPos(ImVec2(nodePosition.x + nodeSize.x / scale + 10.0, nodePosition.y + nodeSize.y / scale / 2.0 - ArrowHeight / 2.0));
  drawLeftArrowView();
}

void HelpService::drawSecondMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Complete the connection", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Drop the connection on the Mix", false);
  ImGui::SetCursorPos(ImVec2(nodePosition.x - ArrowWidth - 10.0, nodePosition.y + (nodeSize.y / scale / 2.0) + (10.0 / scale) - ArrowHeight / 2.0));
  drawRightArrowView();
}

void HelpService::drawOpenShaderInfoView(ImVec2 nodePosition, ImVec2 nodeSize, float scale) {
  ImGui::SetCursorPos(ImVec2(nodePosition.x, nodePosition.y + nodeSize.y / scale + 10.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Open the Effect Settings", false);
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Click on the Mix node to open its settings pane", false);
}

void HelpService::drawEditParametersShaderInfoPane() {
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * (4.0 / 5.0) - 300.0, ImGui::GetWindowHeight() / 3.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Change Parameters", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Drag the slider to change the Mix Amount", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("Most effects have different settings to play with", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::H3Title("You can click on other nodes to change their settings, too", false);
}

void HelpService::drawOpenStageView() {
  // Set the cursor to the bottom right of the window
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? getScaledWindowWidth() / 5 : 0;
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - shaderInfoPaneWidth - 400.0, ImGui::GetWindowHeight() - 400.0));
  float x = ImGui::GetCursorPosX();
  CommonViews::H2Title("Action Bar", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_HELP_OUTLINE, "helpExample"); ImGui::SameLine();
  CommonViews::H3Title("- Turns this tutorial on/off", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_RECYCLING, "resetExample"); ImGui::SameLine();
  CommonViews::H3Title("- Clears the canvas, deleting all nodes and connections", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_CENTER_FOCUS_WEAK, "navigateExample"); ImGui::SameLine();
  CommonViews::H3Title("- Moves the camera so all the nodes are in view", false);
  CommonViews::mSpacing();
  ImGui::SetCursorPosX(x);
  CommonViews::LargeIconButton(ICON_MD_VIEW_AGENDA, "stageModeExample"); ImGui::SameLine();
  CommonViews::H3Title("- Switches to Stage Mode", false); ImGui::SameLine();
  CommonViews::H3BTitle(" - Press this now!", false);
}

void HelpService::drawStageModeHelp() {
  ImVec2 helpSize = ImVec2(ImGui::GetContentRegionAvail().x / 2.0, ImGui::GetContentRegionAvail().y);
  ImGui::BeginChild("stage_mode_help", helpSize, true, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
  CommonViews::H2Title("Stage Mode");
  CommonViews::mSpacing();
  CommonViews::H3Title("This is another way of visualizing your project.");
  CommonViews::mSpacing();
  CommonViews::H3Title("Each square above represents a ");
  ImGui::EndChild();
}
