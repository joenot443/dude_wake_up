//
//  ShaderInfoView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 5/23/24.
//

#include "ShaderInfoView.hpp"
#include "ShaderChainerService.hpp"
#include "imgui.h"
#include "CommonViews.hpp"

void ShaderInfoView::draw() {
  std::shared_ptr<Connectable> selectedShader = ShaderChainerService::getService()->selectedConnectable;
  auto size = ImGui::GetContentRegionAvail();
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0);
  ImGui::BeginChild("ShaderInfoView", ImVec2(size.x - 20.0, size.y - 40.0));
  CommonViews::H2Title("Preview");
  drawPreview();
  CommonViews::H2Title("Settings");
  // Wrap in a Child with 20px padding
  auto innerSize = ImGui::GetContentRegionAvail();
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::InnerChildBackgroundColor.Value);
  ImGui::BeginChild("ShaderInfoSettingsView", ImVec2(innerSize.x, 0.0));
  ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(10.0,  10.0));
  ImGui::BeginChild("ShaderInfoSettingsViewPadding", ImVec2(innerSize.x - 20.0, 0.0));
  selectedShader->drawSettings();
  selectedShader->drawOptionalSettings();
  ImGui::EndChild();
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::EndChild();
}

void ShaderInfoView::drawPreview() {
  std::shared_ptr<Connectable> selectedShader = ShaderChainerService::getService()->selectedConnectable;
  ImTextureID texID = (ImTextureID)(uintptr_t)selectedShader->frame()->getTexture().getTextureData().textureID;
  float availableWidth = ImGui::GetContentRegionAvail().x - 10.0;
  
  float aspectRatio = selectedShader->frame()->getHeight() / selectedShader->frame()->getWidth();
  
  ImGui::Image(texID, ImVec2(availableWidth, availableWidth * aspectRatio));
}
