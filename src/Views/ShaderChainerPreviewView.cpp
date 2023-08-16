//
//  ShaderChainerPreviewView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/16/22.
//

#include "ShaderChainerPreviewView.hpp"
#include "FileSource.hpp"
#include "CommonViews.hpp"
#include "ShaderChainerService.hpp"

void ShaderChainerPreviewView::setup() {
  ofFbo::Settings fboSettings;
  fboSettings.width = width;
  fboSettings.height = height;
  fboSettings.internalformat = GL_RGBA;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  previewFbo.allocate(fboSettings);
  previewFbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
}

void ShaderChainerPreviewView::update() {
  
  
  if (static_cast<int>(previewFbo.getHeight()) != height) {
    setup();
  }
}

void ShaderChainerPreviewView::draw() {
  if (chainer &&
      // If the chainer terminates at an aux/mask,
      // we don't need to display its preview.
      !ShaderChainerService::getService()->terminatesAtAuxPort(chainer)) {
    auto name = formatString("%s##%s", chainer->sourceName.c_str(), chainer->chainerId.c_str());
    if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)){
      width = ImGui::GetWindowWidth() / 6. - 5;
      height = width / 1.3;
      
      if (ImGui::Button(formatString("View##%d", chainer->chainerId.c_str()).c_str())) {
        VideoSourceService::getService()->addOutputWindowForChainer(chainer);
      }
      ImGui::SameLine();
      ImGui::Text(name.c_str());
      ImGui::SameLine();
      CommonViews::ResolutionSelector(chainer);
      
      chainer->fbo.draw(ImGui::GetCursorPosX(), ImGui::GetCursorPosY(), width, height);
      // If the chainer is sourced from a VideoSource, draw playback controls
      if (chainer->source->type == VideoSource_file) {
        // Upcast the chainer source to be a FileSource
        std::shared_ptr<FileSource> fileSource = std::dynamic_pointer_cast<FileSource>(chainer->source);
        if (fileSource != nullptr) {
          // Draw a slider representing the current playback position
          CommonViews::PlaybackSlider(&fileSource->player);
        }
      }
      CommonViews::Spacing(height / 2);
    }
  }
}
