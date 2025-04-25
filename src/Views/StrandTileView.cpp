//
//  StrandTileView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/30/24.
//

#include "StrandTileView.hpp"
#include "LayoutStateService.hpp"
#include "NodeLayoutView.hpp"
#include "imgui.h"
#include "CommonViews.hpp"
#include "ConfigService.hpp"

void StrandTileView::draw(std::shared_ptr<AvailableStrand> strand) {
  ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
  ImGui::BeginChild(strand->id.c_str(), size, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
  ImVec2 origin = ImGui::GetCursorPos();
  ImGui::SetItemAllowOverlap();
  if (ImGui::Button(idString(strand->id).c_str(), size)) {
    NodeLayoutView::getInstance()->clear();
    ConfigService::getService()->loadStrandFile(strand->path);
    LayoutStateService::getService()->showWelcomeScreen = false;
  }
  
  ImGui::SetCursorPos(origin);
  ImVec2 startPos = ImGui::GetCursorPos();
  auto textureId = (ImTextureID)(uintptr_t) strand->fbo->getTexture().texData.textureID;
  ImGui::Image(textureId, size);
  ImGui::SetCursorPos(ImVec2(startPos.x + 5.0, startPos.y));
  ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y / 2.0);
  CommonViews::H4Title(strand->name, false);
  ImGui::EndChild();
}

