//
//  VideoSourceBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "VideoSourceBrowserView.hpp"
#include "AvailableVideoSource.hpp"
#include "FontService.hpp"
#include "LibraryService.hpp"
#include "ofMain.h"
#include "TileBrowserView.hpp"
#include "VideoSourceService.hpp"

void VideoSourceBrowserView::setup()
{
  VideoSourceService::getService()->subscribeToAvailableVideoSourceUpdates(
      [this]()
      { refreshSources(); });
  ofAddListener(LibraryService::getService()->thumbnailNotification, this, &VideoSourceBrowserView::refreshSources);

  refreshSources();

  // Initialize the search results view
  searchResultsTileBrowserView = TileBrowserView(searchTileItems);
}

void VideoSourceBrowserView::refreshSources()
{
  shaderItems.clear();
  libraryItems.clear();
  webcamItems.clear();
  auto sources = VideoSourceService::getService()->availableVideoSources();
  for (auto source : sources)
  {
    if (!source->hasPreview)
    {
      source->generatePreview();
    }

    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [source]()
    {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("VideoSource", &source.get()->availableVideoSourceId,
                                  sizeof(std::string));
        ImGui::Text("%s", source->sourceName.c_str());
        ImGui::EndDragDropSource();
      }
    };
    ImTextureID textureId = (ImTextureID)(uint64_t) source->preview->texData.textureID;
    auto tileItem = std::make_shared<TileItem>(source->sourceName, textureId, 0, dragCallback, source->category);

    if (source->type == VideoSource_shader || 
        source->type == VideoSource_text ||
        source->type == VideoSource_multi ||
        source->type == VideoSource_icon)
    {
      shaderItems.push_back(tileItem);
    }
    else if (source->type == VideoSource_library)
    {
      if (std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(source) != nullptr)
      {
        auto librarySource = std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(source);
        auto libraryTileItem = std::make_shared<LibraryTileItem>(source->sourceName, textureId, 0, librarySource->libraryFile, dragCallback, librarySource->libraryFile->category);
        libraryItems.push_back(libraryTileItem);
      }
    }
    else if (source->type == VideoSource_webcam)
    {
      webcamItems.push_back(tileItem);
    }
  }
  // Sort the shader items by name
  std::sort(shaderItems.begin(), shaderItems.end(),
            [](std::shared_ptr<TileItem> a, std::shared_ptr<TileItem> b)
            { return a->name < b->name; });
  // Sort the library items by name
  std::sort(libraryItems.begin(), libraryItems.end(),
            [](std::shared_ptr<LibraryTileItem> a, std::shared_ptr<LibraryTileItem> b)
            { return a->name < b->name; });

  // Sort the webcam items by name
  std::sort(webcamItems.begin(), webcamItems.end(),
            [](std::shared_ptr<TileItem> a, std::shared_ptr<TileItem> b)
            { return a->name < b->name; });

  tileBrowserView = TileBrowserView(shaderItems);
  fileBrowserView.setup();
}

void VideoSourceBrowserView::update()
{
//  if (searchDirty) {
//    // Filter tileItems based on searchQuery
//    auto sources = VideoSourceService::getService()->availableVideoSources();
//    std::vector<std::shared_ptr<TileItem>> filteredItems;
//
//    // Convert the searchQuery to lowercase for case-insensitive comparison
//    std::string searchQueryLower = searchQuery;
//    std::transform(searchQueryLower.begin(), searchQueryLower.end(), searchQueryLower.begin(),
//                   [](unsigned char c){ return std::tolower(c); });
//
//    for (auto& source : sources) {
//        // Convert source name to lowercase for case-insensitive comparison
//        std::string sourceNameLower = source->sourceName;
//        std::transform(sourceNameLower.begin(), sourceNameLower.end(), sourceNameLower.begin(),
//                       [](unsigned char c){ return std::tolower(c); });
//
//        if (sourceNameLower.find(searchQueryLower) != std::string::npos) {
//            ImTextureID textureId = (ImTextureID)(uint64_t) source->preview->texData.textureID;
//            auto tileItem = std::make_shared<TileItem>(source->sourceName, textureId, 0, nullptr, source->category);
//            filteredItems.push_back(tileItem);
//        }
//    }
//    searchResultsTileBrowserView.setTileItems(filteredItems);
//    searchDirty = false;
//  }
}

void VideoSourceBrowserView::drawLibraryHeader() {}

void VideoSourceBrowserView::draw()
{
//  char buffer[256];
//  strncpy(buffer, searchQuery.c_str(), sizeof(buffer));
//  if (ImGui::InputText("Search", buffer, sizeof(buffer))) {
//    searchQuery = std::string(buffer);
//    searchDirty = true;
//  }
//
//  if (searchQuery.length() != 0) {
//    if (ImGui::BeginTabBar("VideoSourceBrowser", ImGuiTabBarFlags_None)) {
//      if (ImGui::BeginTabItem("Search")) {
//        searchResultsTileBrowserView.draw();
//      }
//      ImGui::EndTabBar();
//    }
//    return;
//  }

  if (ImGui::BeginTabBar("VideoSourceBrowser", ImGuiTabBarFlags_None))
  {
    if (ImGui::BeginTabItem("Generated"))
    {
      tileBrowserView = TileBrowserView(shaderItems);
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Webcam"))
    {
      tileBrowserView = TileBrowserView(webcamItems);
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Library"))
    {
      tileBrowserView = TileBrowserView(libraryItems);
      tileBrowserView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Local Files"))
    {
      fileBrowserView.draw();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}
