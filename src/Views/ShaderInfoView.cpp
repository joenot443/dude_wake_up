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
  CommonViews::H3Title("Preview");
  CommonViews::sSpacing();
  drawPreview();
  CommonViews::H3Title("Settings");
  ImGui::Separator();
  selectedShader->drawSettings();
}

void ShaderInfoView::drawPreview() {
  std::shared_ptr<Connectable> selectedShader = ShaderChainerService::getService()->selectedConnectable;
  ImTextureID texID = (ImTextureID)(uintptr_t)selectedShader->frame()->getTexture().getTextureData().textureID;
  float availableWidth = ImGui::GetContentRegionAvail().x;
  
  float aspectRatio = selectedShader->frame()->getHeight() / selectedShader->frame()->getWidth();
  
  ImGui::Image(texID, ImVec2(availableWidth, availableWidth * aspectRatio));
}
