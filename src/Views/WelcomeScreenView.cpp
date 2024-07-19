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
#include "imgui.h"

void WelcomeScreenView::setup() {
  recentBrowserWindow.setup();
  templateBrowserWindow.setup();
}

void WelcomeScreenView::update() {
  
}

void WelcomeScreenView::draw() {
  ImVec2 helpSize = ImVec2(ImGui::GetWindowWidth() / 2.0, ImGui::GetWindowHeight() / 2.0);
  ImVec2 helpPos = ImVec2(ImGui::GetWindowWidth() / 4.0, ImGui::GetWindowHeight() / 4.0);
  
  ImGui::SetNextWindowPos(helpPos);
  ImGui::SetNextWindowSize(helpSize);
  
  ImGui::OpenPopup("##welcomeScreen");
  
  if (ImGui::BeginPopupModal("##welcomeScreen", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {
    ImVec2 containerSize = ImVec2(ImGui::GetContentRegionAvail().x * 0.8, ImGui::GetContentRegionAvail().y * 0.2);
    float containerOffset = ImGui::GetContentRegionAvail().x * 0.1;
    float titleWidth = 557.0; // Tested
    CommonViews::mSpacing();
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - titleWidth) / 2.0);
    CommonViews::H1Title("welcome to nottawa", false);
    
    CommonViews::mSpacing();
    
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 525.0)/ 2.0);
    
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
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y -
                         containerSize.y * 2 - 130.0);
    
    
    ImGui::SetCursorPosX(containerOffset);
    // Templates container
    CommonViews::H3Title("Templates", false);
    CommonViews::sSpacing();
    ImGui::SetCursorPosX(containerOffset);
    ImGui::BeginChild("##templatesContainer", containerSize);
    templateBrowserWindow.draw();
    ImGui::EndChild();
    
    CommonViews::lSpacing();
    
    ImGui::SetCursorPosX(containerOffset);
    // Recent container
    CommonViews::H3Title("Recent", false);
    CommonViews::sSpacing();
    ImGui::SetCursorPosX(containerOffset);
    ImGui::BeginChild("##recentsContainer", containerSize, ImGuiWindowFlags_HorizontalScrollbar);
    recentBrowserWindow.draw();
    ImGui::EndChild();
    
    CommonViews::H4Title("Always show welcome screen at launch?", false);
    ImGui::SameLine();
    ImGui::Checkbox("##welcome", &LayoutStateService::getService()->welcomeScreenEnabled);
    
    ImGui::EndPopup();
  }
}


void WelcomeScreenView::newWorkspace() {
  LayoutStateService::getService()->showWelcomeScreen = false;
}

void WelcomeScreenView::openWorkspace() {
  if (ConfigService::getService()->loadWorkspaceDialogue()) {
    LayoutStateService::getService()->showWelcomeScreen = false;
  }
}
