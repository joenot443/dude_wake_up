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

void VideoSource::drawMaskSettings() {
 CommonViews::ShaderCheckbox(settings->maskEnabled);
 if (settings->maskEnabled->boolValue) {
   CommonViews::Slider("Mask Tolerance", "##maskTolerance", settings->maskTolerance);
   CommonViews::ShaderColor(settings->maskColor);
   CommonViews::ShaderCheckbox(settings->invert);
 }
}

void VideoSource::drawPreview(ImVec2 pos, float scale) {
  if (!fbo->isAllocated()) return;
  
  ofTexture tex = fbo->getTexture();
  if (tex.isAllocated()) {
    ImTextureID texID = (ImTextureID)(uintptr_t)tex.getTextureData().textureID;
    ImGui::Image(texID, LayoutStateService::getService()->previewSize(scale));
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
