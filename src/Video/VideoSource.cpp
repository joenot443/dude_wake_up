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
    ImGui::Image(texID, size);
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
