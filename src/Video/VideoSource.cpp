//
//  VideoSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/7/24.
//

#include <stdio.h>
#include "VideoSource.hpp"
#include "CommonViews.hpp"
#include "ShaderSource.hpp"
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
  ImVec2 cursorPos = ImGui::GetCursorPos();
  if (tex.isAllocated()) {
    ImTextureID texID = (ImTextureID)(uintptr_t)tex.getTextureData().textureID;

    float fboW = fbo->getWidth();
    float fboH = fbo->getHeight();
    if (fboW <= 0 || fboH <= 0) return;

    float fboAspect = fboW / fboH;
    float boxAspect = size.x / size.y;

    ImVec2 drawSize;
    if (fboAspect > boxAspect) {
      drawSize.x = size.x;
      drawSize.y = size.x / fboAspect;
    } else {
      drawSize.y = size.y;
      drawSize.x = size.y * fboAspect;
    }

    ImVec2 offset((size.x - drawSize.x) * 0.5f, (size.y - drawSize.y) * 0.5f);
    ImVec2 topLeft = cursorPos + offset;

    ImGui::GetWindowDrawList()->AddImageRounded(
      texID,
      topLeft,
      topLeft + drawSize,
      ImVec2(0,0),
      ImVec2(1,1),
      IM_COL32_WHITE,
      8.0
    );
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

std::shared_ptr<Settings> VideoSource::settingsRef() {
  if (type == VideoSource_shader) {
    std::shared_ptr<ShaderSource> shaderSource = std::dynamic_pointer_cast<ShaderSource>(shared_from_this());
    if (shaderSource && shaderSource->shader) {
      return std::dynamic_pointer_cast<Settings>(shaderSource->shader->settings);
    }
    return nullptr;
  } else {
    return std::dynamic_pointer_cast<Settings>(settings);
  }
}
