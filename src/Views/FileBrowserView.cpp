//
//  FileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "FileBrowserView.hpp"
#include "VideoSourceService.hpp"
#include "Console.hpp"
#include "BookmarkService.hpp"
#include "File.hpp"
#include "AvailableStrand.hpp"
#include "AvailableVideoSource.hpp"
#include "ofMain.h"
#include "ConfigService.hpp"
#include "LayoutStateService.hpp"
#include "ConfigService.hpp"
#include "ofxImGui.h"

void FileBrowserView::refresh()
{
  if (currentDirectory == nullptr) return;

  files.clear();
  currentDirectory->open(currentDirectory->getAbsolutePath());
  if (!currentDirectory->isDirectory() || !currentDirectory->canRead()) { return; }

  try {
    currentDirectory->listDir();
  } catch (std::exception &) {
    log("Couldn't open directory for FileBrowserView");
    return;
  }

  currentDirectory->sort();

  for (int i = 0; i < currentDirectory->size(); i++)
  {
    bool isDirectory = currentDirectory->getFile(i).isDirectory();
    files.push_back(File(currentDirectory->getPath(i), isDirectory));
  }
  std::vector<std::shared_ptr<TileItem>> tileItems = {};

  for (auto file : files)
  {
    std::shared_ptr<AvailableVideoSource> availableSource;
    if (isVideoFile(file.path)) {
      availableSource = std::make_shared<AvailableVideoSourceFile>(
                                                                   file.name, file.path);
    } else if (isImageFile(file.path)) {
      availableSource = std::make_shared<AvailableVideoSourceImage>(file.name, file.path);
      availableSource->generatePreview();
    } else {
      continue;
    }

    // Open the file and get the first frame
    sources.push_back(availableSource);
    // Create a closure which will be called when the tile is clicked
    std::function<void(std::string)> dragCallback = [availableSource](std::string tileId)
    {
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        ImGui::SetDragDropPayload("VideoSource", &availableSource.get()->availableVideoSourceId,
                                  sizeof(std::string));
        ImGui::PushID(tileId.c_str());
        ImGui::Text("%s", availableSource->sourceName.c_str());
        ImGui::PopID();
        ImGui::EndDragDropSource();
      }
    };
    ImTextureID textureId = (ImTextureID)(uint64_t) availableSource->preview->texData.textureID;
    std::shared_ptr<TileItem> tileItem = std::make_shared<TileItem>(file.name, textureId, 0, dragCallback, "", TileType_File, ShaderTypeNone);

    if (isVideoFile(file.path))
      previewQueue.push(std::make_pair(std::dynamic_pointer_cast<AvailableVideoSourceFile>(availableSource), tileItem));

    tileItems.push_back(tileItem);
  }
  VideoSourceService::getService()->addAvailableVideoSources(sources);
  listBrowserView.tileItems = tileItems;
  tileBrowserView.setTileItems(tileItems);
  update();
}

void FileBrowserView::setup()
{
  // For our Sources (Video file) browser, let's use the LibraryPath from the LayoutStateService.
  if (LayoutStateService::getService()->libraryPath.length() > 0) {
    currentDirectory = std::make_shared<ofDirectory>(LayoutStateService::getService()->libraryPath);
  }
  refresh();
}

void FileBrowserView::loadDirectory(std::string directory)
{
  LayoutStateService::getService()->updateLibraryPath(directory);
  currentDirectory = std::make_shared<ofDirectory>(directory);
  refresh();
}

void FileBrowserView::update() {
  //  return;

  if (previewQueue.empty()) return;

  auto& [source, tile] = previewQueue.front();

  if (!videoPlayer.isLoaded()) {
    if (!videoPlayer.load(source->path)) {
      ofLogError("FileBrowserView") << "Failed to load video file: " << source->path;
      previewQueue.pop();
      return;
    }
    videoPlayer.play();
    videoPlayer.setVolume(0.0);
  }

  videoPlayer.update();

  if (source->hasFailedToLoad(videoPlayer)) {
    previewQueue.pop();
    return;
  }

  if (source->canGeneratePreview(videoPlayer)) {
    source->generatePreview(videoPlayer);
    tile->textureID = (ImTextureID) source->preview->texData.textureID;
    previewQueue.pop();
    videoPlayer.close();
  }
}

// Display the Files in the files vector as items in a ListBox.
// If a file is selected, update settings.selectedFile
void FileBrowserView::draw()
{
  // Add a button to open the file browser to choose the current directory
  if (ImGui::Button("Open"))
  {
    ofFileDialogResult result = ofSystemLoadDialog("Choose Directory", true);
    if (result.bSuccess)
    {
      // Save the new path in LayoutStateService
      LayoutStateService::getService()->updateLibraryPath(result.filePath);
      currentDirectory = std::make_shared<ofDirectory>(result.filePath);
      refresh();
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("Show in Finder"))
  {
    // macOS
    std::string command = "open " + currentDirectory->getAbsolutePath();
    system(command.c_str());
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh"))
  {
    refresh();
  }

  if (currentDirectory != nullptr) {
    ImGui::SameLine();
    ImGui::Text("%s", currentDirectory->getAbsolutePath().c_str());
    tileBrowserView.draw();
  } else {
    ImGui::PushFont(FontService::getService()->current->sm);
    ImGui::Text("Press Open to choose a source directory");
    ImGui::PopFont();
  }
}
