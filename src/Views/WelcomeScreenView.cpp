//
//  WelcomeScreenView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/28/24.
//

#include "ofMain.h"
#include "NodeLayoutView.hpp"
#include "WelcomeScreenView.hpp"
#include "StrandBrowserWindow.hpp"
#include "LayoutStateService.hpp"
#include "CommonViews.hpp"
#include "FontService.hpp"
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


void WelcomeScreenView::draw()
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0, 24.0));
  /* ---------- 1. Modal placement ---------- */
  float windowW = ImGui::GetWindowWidth() * 0.60f;
  float windowH = ImGui::GetWindowHeight() * 0.80f;
  
  const ImVec2 centre{
    (ImGui::GetWindowWidth()  - windowW) * 0.5f,
    (ImGui::GetWindowHeight() - windowH) * 0.5f
  };
  
  ImGui::SetNextWindowPos(centre);
  ImGui::SetNextWindowSize({static_cast<float>(windowW + 48.0), static_cast<float>(windowH + 48.0)});
  ImGui::OpenPopup("##welcomeScreen");
  
  if (!ImGui::BeginPopupModal("##welcomeScreen", nullptr,
                              ImGuiWindowFlags_NoDecoration |
                              ImGuiWindowFlags_AlwaysAutoResize))
    return;
  
  /* ---------- 2. Header video -------------- */
  ImTextureID texID = (ImTextureID)(uintptr_t)player.getTexture()
    .getTextureData().textureID;
  const float headerH = windowW * 0.30f;                       // 16:5‑ish ratio
  
  // Remember where the image will be drawn so we can overlay later.
  const ImVec2 imgTL = ImGui::GetCursorScreenPos();            // top‑left
  const ImVec2 imgBR = {static_cast<float>(imgTL.x + windowW), imgTL.y + headerH}; // bottom‑right
  
  ImGui::Image(texID, {windowW, headerH});
  
  /* ---------- 3. Toggle overlay ------------ */
  {
    // position about 10 px inset from the bottom‑left of the image
    const float inset      = 10.0f;
    const ImVec2 overlayPos{imgTL.x + inset,
      imgBR.y - inset - 28.0f};        // 28‑px tall pill
    
    ImGui::SetCursorScreenPos(overlayPos);
    
    // Semi‑translucent pill background & rounded corners
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0, 0, 0, 128).Value);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
    
    // Make the child exactly wide enough for the text + checkbox
    ImGui::BeginChild("##welcomeToggle", ImVec2(260, 34),
                      /*border=*/false, ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoDecoration);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 255, 220).Value);
    ImGui::Checkbox(" Always show at launch",
                    &LayoutStateService::getService()->welcomeScreenEnabled);
    ImGui::PopStyleColor();
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  }
  
  /* ---------- 3a. Demo Button Overlay ------------ */
  {
    // position about 10 px inset from the bottom‑right of the image
    const float inset = 10.0f;
    const ImVec2 buttonSize = {120.0f, 34.0f}; // Button size
    const ImVec2 buttonPos = {imgBR.x - inset - buttonSize.x,
                              imgBR.y - inset - buttonSize.y};
                              
    ImGui::SetCursorScreenPos(buttonPos);
    
    // Style: 70% Opaque white border
    ImGui::PushStyleColor(ImGuiCol_Border, ImColor(255, 255, 255, (int)(255 * 0.7)).Value);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f); // Match pill rounding

    if (ImGui::Button("Try this out", buttonSize)) {
        openDemo();
    }

    ImGui::PopStyleVar(2); // Pop FrameBorderSize and FrameRounding
    ImGui::PopStyleColor(); // Pop Border color
  }
  
  ImGui::Spacing();   // breathing room below the video
  
  /* ---------- 4. Action buttons ------------ */
  constexpr float BTN_W = 180.f, BTN_H = 48.f, GAP = 32.f;
  ImGui::PushFont(FontService::getService()->current->h3);
  
  const float rowW = 3 * BTN_W + 2 * GAP;
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - rowW) * 0.5f);
  
  ImGui::BeginGroup();
  if (ImGui::Button("Resume Workspace", {BTN_W, BTN_H}))
    LayoutStateService::getService()->showWelcomeScreen = false;
  ImGui::SameLine(0, GAP);
  if (ImGui::Button("New Workspace", {BTN_W, BTN_H}))
    newWorkspace();
  ImGui::SameLine(0, GAP);
  if (ImGui::Button("Open Workspace", {BTN_W, BTN_H}))
    openWorkspace();
  ImGui::EndGroup();
  
  ImGui::PopFont();
  ImGui::Spacing(); ImGui::Spacing();
  
  /* ---------- 5. Two‑column browsers -------- */
  const float colGap    = ImGui::GetStyle().ItemSpacing.x * 2.0f;
  const float colWidth  = (ImGui::GetContentRegionAvail().x - colGap) * 0.5f;
  const float colHeight = ImGui::GetContentRegionAvail().y - 30.0f;
  
  // Templates
  ImGui::BeginGroup();
  CommonViews::H3Title("Templates", false);
  
  ImGui::BeginChild("##templatesContainer",
                    {colWidth, colHeight}, false);
  templateBrowserWindow.draw();
  ImGui::EndChild();
  ImGui::EndGroup();
  
  // Recents
  ImGui::SameLine(0, colGap);
  ImGui::BeginGroup();
  CommonViews::H3Title("Recent", false);
  ImGui::BeginChild("##recentsContainer",
                    {colWidth, colHeight}, false);
  recentBrowserWindow.draw();
  ImGui::EndChild();
  ImGui::EndGroup();
  
  /* ---------- 6. Close modal --------------- */
  ImGui::EndPopup();
  ImGui::PopStyleVar();
}




void WelcomeScreenView::newWorkspace() {
  NodeLayoutView::getInstance()->clear();
  LayoutStateService::getService()->showWelcomeScreen = false;
}

void WelcomeScreenView::openWorkspace() {
  if (ConfigService::getService()->loadWorkspaceDialogue()) {
    LayoutStateService::getService()->showWelcomeScreen = false;
    LayoutStateService::getService()->helpEnabled = false;
  }
}

void WelcomeScreenView::openDemo() {
  // TODO: Implement loading the demo workspace
  LayoutStateService::getService()->helpEnabled = false;
  NodeLayoutView::getInstance()->clear();
  ConfigService::getService()->loadStrandFile(StrandService::getService()->demoStrand->path);
  LayoutStateService::getService()->showWelcomeScreen = false;
}
