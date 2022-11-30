//
//  VideoSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "VideoSourceBrowserView.hpp"
#include "FontService.hpp"
#include "VideoSourceService.hpp"


void VideoSourceBrowserView::setup() {}

void VideoSourceBrowserView::update() {

}

void VideoSourceBrowserView::draw() {
    drawVideoSourceSelector();
    drawVideoSourceFrame();
}

// Draw a list of video sources as items in a ListBox
void VideoSourceBrowserView::drawVideoSourceSelector() {
  auto sources = VideoSourceService::getService()->videoSources();
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Video Source");
  ImGui::PopFont();
  
  
  if (ImGui::BeginListBox("##videosources")) {
    for (int i = 0; i < sources.size(); i++) {
      if (ImGui::Selectable(sources[i]->sourceName.c_str())) {
        selectedVideoSource = sources[i];
      }
    }
    ImGui::EndListBox();
  }
}

void VideoSourceBrowserView::drawVideoSourceFrame() {
//  if (selectedVideoSource) {
//    videoSourceFbo.begin();
//    selectedVideoSource->frameTexture->draw(0, 480 * 2, 640 * 2, -480 * 2);
//    videoSourceFbo.end();
//    
//    ImTextureID textureID = (ImTextureID)(uintptr_t)videoSourceFbo.getTexture().getTextureData().textureID;
//    auto size = ImGui::GetItemRectSize();
//    ImGui::Image(textureID, ImVec2(320, 240));
//  }
}
