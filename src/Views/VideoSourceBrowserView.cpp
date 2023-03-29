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
    
    if (source->type == VideoSource_shader) {
      shaderItems.push_back(tileItem);
    } else if (source->type == VideoSource_webcam) {
      webcamItems.push_back(tileItem);
    } else if (source->type == VideoSource_text) {
      textItems.push_back(tileItem);
    }
  }
  
  tileBrowserView = TileBrowserView(shaderItems);
  fileBrowserView.setup();
}

void VideoSourceBrowserView::update() {}

void VideoSourceBrowserView::draw() {
  CommonViews::H3Title("Sources");
  
  if (ImGui::BeginTabBar("VideoSourceBrowser", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Generated"))
    {
      tileBrowserView.tileItems = shaderItems;
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Webcam"))
    {
      tileBrowserView.tileItems = webcamItems;
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Text"))
    {
      tileBrowserView.tileItems = textItems;
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("File Browser"))
    {
      fileBrowserView.draw();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}
