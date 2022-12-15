//
//  VideoSourcePreviewView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/16/22.
//

#include "VideoSourcePreviewView.hpp"

void VideoSourcePreviewView::setup() {
  ofFbo::Settings fboSettings;
  fboSettings.width = 320;
  fboSettings.height = 240;
  fboSettings.internalformat = GL_RGB;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  previewFbo.allocate(fboSettings);
}

void VideoSourcePreviewView::update() {
  
}

void VideoSourcePreviewView::draw() {
  // Surround the image with a 10px grey border
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 10.0f);
  
  
  if (videoSource && videoSource->frameTexture) {
    ImGui::Text(videoSource->sourceName.c_str());

    previewFbo.begin();
    videoSource->frameTexture->draw(0, 0, 320, 240);
    previewFbo.end();
    
    auto id = previewFbo.getTexture().getTextureData().textureID;
    ImGui::Image((void*) id, ImVec2(320, 240));
    ImGui::SameLine();
    if (ImGui::Button(formatString("View##%s", videoSource->sourceName.c_str()).c_str())) {
      VideoSourceService::getService()->addOutputWindowForVideoSource(videoSource);
    }
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
}
