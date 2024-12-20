//
//  VideoSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/7/24.
//

#include <stdio.h>
#include "VideoSource.hpp"
#include "CommonViews.hpp"
#include "LayoutStateService.hpp"

void VideoSource::drawPreview(ImVec2 pos, float scale) {
  if (!fbo->isAllocated()) return;
  
  ofTexture tex = fbo->getTexture();
  if (tex.isAllocated()) {
    ImTextureID texID = (ImTextureID)(uintptr_t)tex.getTextureData().textureID;
    ImGui::Image(texID, LayoutStateService::getService()->previewSize(scale));
  }
}

void VideoSource::drawPreviewSized(ImVec2 size) {
  if (!fbo->isAllocated()) return;
  
  ofTexture tex = fbo->getTexture();
  if (tex.isAllocated()) {
    ImTextureID texID = (ImTextureID)(uintptr_t)tex.getTextureData().textureID;
    ImGui::GetWindowDrawList()->AddImageRounded(texID, ImGui::GetCursorPos() - ImVec2(0.0, size.y), ImGui::GetCursorPos() + ImVec2(size.x, 0.0), ImVec2(0,0), ImVec2(1,1), IM_COL32_WHITE, 8.0);
  }
}

void VideoSource::drawOptionalSettings() {
    optionalShadersHelper.drawOptionalSettings();
}

void VideoSource::applyOptionalShaders() {
    optionalShadersHelper.applyOptionalShaders(fbo, optionalFbo);
}

void VideoSource::generateOptionalShaders() {
    optionalShadersHelper.generateOptionalShaders();
}
