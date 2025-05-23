//
//  WelcomeScreenView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/28/24.
//

#include "WelcomeScreenView.hpp"
#include "LayoutStateService.hpp"
#include "CommonViews.hpp"
#include "FontService.hpp"
#include "NodeLayoutView.hpp"
#include "imgui.h"

void WelcomeScreenView::setup() {
  recentBrowserWindow.setup();
  templateBrowserWindow.setup();
  ofDirectory dir("images/intro");
  dir.listDir();
  ofFile file = dir.getFile(0);
  player.load(file.pathFS());
  player.play();
}

void WelcomeScreenView::update() {
  player.update();
}

void WelcomeScreenView::draw() {
  ImVec2 helpSize = ImVec2(ImGui::GetWindowWidth() / 2.0, ImGui::GetWindowHeight() / 2.0);
  ImVec2 helpPos = ImVec2(ImGui::GetWindowWidth() / 4.0, ImGui::GetWindowHeight() / 4.0);
  
  ImGui::SetNextWindowPos(helpPos);
  ImGui::SetNextItemWidth(helpSize.x);
  
  ImGui::OpenPopup("##welcomeScreen");
  
  if (ImGui::BeginPopupModal("##welcomeScreen", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {
    ImVec2 containerSize = ImVec2(ImGui::GetContentRegionAvail().x * 0.8, ImGui::GetContentRegionAvail().y * 0.2);
    float containerOffset = ImGui::GetContentRegionAvail().x * 0.1;
    ImTextureID texID = (ImTextureID)(uintptr_t)player.getTexture().getTextureData().textureID;
    ImGui::Image(texID, ImVec2(helpSize.x, helpSize.x * 0.304));
    
    ImGui::PushFont(FontService::getService()->h3);
    
    if (ImGui::Button("Resume Workspace", ImVec2(150, 50))) {
      LayoutStateService::getService()->showWelcomeScreen = false;
    }
    
    ImGui::SameLine(0, 50);
    
    if (ImGui::Button("New Workspace", ImVec2(150, 50))) {
      newWorkspace();
    }
    
    ImGui::SameLine(0, 50);
    
    if (ImGui::Button("Open Workspace", ImVec2(150, 50))) {
      openWorkspace();
    }
    ImGui::PopFont();
    
    ImGui::SetCursorPosX(containerOffset);
    // Templates container
    CommonViews::H3Title("Templates", false);
    ImGui::SetCursorPosX(containerOffset);
    ImGui::BeginChild("##templatesContainer", containerSize);
    templateBrowserWindow.draw();
    ImGui::EndChild();
    
    ImGui::SetCursorPosX(containerOffset);
    // Recent container
    CommonViews::H3Title("Recent", false);
    ImGui::SetCursorPosX(containerOffset);
    ImGui::BeginChild("##recentsContainer", containerSize, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
    recentBrowserWindow.draw();
    ImGui::EndChild();
    
    CommonViews::H4Title("Always show welcome screen at launch?", false);
    ImGui::SameLine();
    ImGui::Checkbox("##welcome", &LayoutStateService::getService()->welcomeScreenEnabled);
    
    ImGui::EndPopup();
  }
}


void WelcomeScreenView::newWorkspace() {
  NodeLayoutView::getInstance()->clear();
  LayoutStateService::getService()->showWelcomeScreen = false;
}

void WelcomeScreenView::openWorkspace() {
  if (ConfigService::getService()->loadWorkspaceDialogue()) {
    LayoutStateService::getService()->showWelcomeScreen = false;
  }
}
