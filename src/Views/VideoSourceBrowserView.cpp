//
//  VideoSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "VideoSourceBrowserView.hpp"
#include "AvailableVideoSource.hpp"
#include "FontService.hpp"
#include "TileBrowserView.hpp"
#include "VideoSourceService.hpp"

void VideoSourceBrowserView::setup() {
  auto sources = VideoSourceService::getService()->availableVideoSources();

  std::vector<TileItem> tileItems = {};
  for (auto source : sources) {

    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [source]() {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("VideoSource", source.get(),
                                  sizeof(AvailableVideoSource));
        ImGui::Text("%s", source->sourceName.c_str());
        ImGui::EndDragDropSource();
      }
    };
    
    auto textureId = GetImTextureID(*source->preview.get());
    TileItem tileItem = TileItem(source->sourceName, textureId, 0, dragCallback);

    tileItems.push_back(tileItem);
  }

  tileBrowserView = TileBrowserView(tileItems);
}

void VideoSourceBrowserView::update() {}

void VideoSourceBrowserView::draw() {
  drawVideoSourceSelector();
  drawVideoSourceFrame();
}

// Draw a list of video sources as items in a ListBox
void VideoSourceBrowserView::drawVideoSourceSelector() {

  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("Video Source");
  ImGui::PopFont();

  tileBrowserView.draw();

  // if (ImGui::BeginListBox("##videosources")) {
  //   for (int i = 0; i < sources.size(); i++) {
  //     if (ImGui::Selectable(sources[i]->sourceName.c_str())) {
  //     }
  //     if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
  //       // Set payload to carry the index of our item (could be anything)
  //       auto source = sources[i];
  //       ImGui::SetDragDropPayload("VideoSource", source.get(),
  //                                 sizeof(AvailableVideoSource));
  //       ImGui::Text("%s", source->sourceName.c_str());
  //       ImGui::EndDragDropSource();
  //     }
  //   }
  //   ImGui::EndListBox();
  // }
}

void VideoSourceBrowserView::drawVideoSourceFrame() {
  //  if (selectedVideoSource) {
  //    videoSourceFbo.begin();
  //    selectedVideoSource->frameTexture->draw(0, 480 * 2, 640 * 2, -480 * 2);
  //    videoSourceFbo.end();
  //
  //    ImTextureID textureID =
  //    (ImTextureID)(uintptr_t)videoSourceFbo.getTexture().getTextureData().textureID;
  //    auto size = ImGui::GetItemRectSize();
  //    ImGui::Image(textureID, ImVec2(320, 240));
  //  }
}
