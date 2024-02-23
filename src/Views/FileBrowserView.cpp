//
//  FileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "FileBrowserView.hpp"
#include "VideoSourceService.hpp"
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
  files.clear();

  currentDirectory.open(currentDirectory.getAbsolutePath());
  currentDirectory.sort();

  for (int i = 0; i < currentDirectory.size(); i++)
  {
    bool isDirectory = currentDirectory.getFile(i).isDirectory();
    files.push_back(File(currentDirectory.getPath(i), isDirectory));
  }
  std::vector<TileItem> tileItems = {};

  for (auto file : files)
  {
    if (type == FileBrowserType_Source)
    {
      std::shared_ptr<AvailableVideoSource> availableSource;
      if (isVideoFile(file.path)) {
        availableSource = std::make_shared<AvailableVideoSourceFile>(
                                                                     file.name, file.path);
      } else if (isImageFile(file.path)) {
        availableSource = std::make_shared<AvailableVideoSourceImage>(file.name, file.path);
      } else {
        continue;
      }
      
      // Open the file and get the first frame
      sources.push_back(availableSource);
      // Create a closure which will be called when the tile is clicked
      std::function<void()> dragCallback = [availableSource]()
      {
        // Create a payload to carry the video source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
          ImGui::SetDragDropPayload("VideoSource", &availableSource->availableVideoSourceId,
                                    sizeof(std::string));
          ImGui::Text("%s", availableSource->sourceName.c_str());
          ImGui::EndDragDropSource();
        }
      };
      TileItem tileItem = TileItem(file.name, 0, 0, dragCallback);
      tileItems.push_back(tileItem);
    }
    else if (type == FileBrowserType_JSON)
    {
      // Validate the file is a json file
      if (file.name.find(".json") == std::string::npos)
      {
        continue;
      }
      // Validate that the file is a valid Strand
      if (ConfigService::getService()->validateStrandJson(file.path) == false)
      {
        continue;
      }

      auto availableStrand =
          ConfigService::getService()->availableStrandFromPath(file.path);

      // Create a closure which will be called when the tile is dragged
      std::function<void()> dragCallback = [availableStrand]()
      {
        // Create a payload to carry the video source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
          // Set payload to carry the index of our item (could be anything)
          ImGui::SetDragDropPayload("AvailableStrand", &availableStrand,
                                    sizeof(availableStrand));
          ImGui::Text("%s", availableStrand.name.c_str());
          ImGui::EndDragDropSource();
        }
      };

      TileItem tileItem = TileItem(availableStrand.name, 0, 0, dragCallback);
      tileItems.push_back(tileItem);
    }
  }
  VideoSourceService::getService()->addAvailableVideoSources(sources);
  listBrowserView.tileItems = tileItems;
}

void FileBrowserView::setup()
{
  // For our Sources (Video file) browser, let's use the LibraryPath from the LayoutStateService.
  if (type == FileBrowserType_Source)
  {
    currentDirectory = ofDirectory(LayoutStateService::getService()->libraryPath);
  }
  else
  {
    currentDirectory = ofDirectory(ConfigService::getService()->nottawaFolderFilePath());
    ConfigService::getService()->subscribeToConfigUpdates([this]()
                                                          { refresh(); });
  }

  refresh();
}

// Get a list of all files in the current directory.
// Create a File for each file and add it to the files vector.
void FileBrowserView::update() {}

// Display the Files in the files vector as items in a ListBox.
// If a file is selected, update settings.selectedFile
void FileBrowserView::draw()
{
  // If we're in the JSON browser, just draw the list browser view
  if (type == FileBrowserType_JSON)
  {
    listBrowserView.draw();
    return;
  }

  // Add a button to change the current directory to the parent directory.
  if (ImGui::Button("Go Up"))
  {
    ofDirectory dir;
    dir.open(currentDirectory);
    // Get the parent directory of the current directory by removing the last
    // directory from the path.
    auto newPath = currentDirectory.getAbsolutePath().substr(
        0, dir.getAbsolutePath().find_last_of("/\\"));
    // Save the new path in LayoutStateService
    LayoutStateService::getService()->updateLibraryPath(newPath);
    currentDirectory = ofDirectory(newPath);
    refresh();
  }
  ImGui::SameLine();

  // Add a button to open the file browser to choose the current directory
  if (ImGui::Button("Open"))
  {
    ofFileDialogResult result = ofSystemLoadDialog("Choose Directory", true);
    if (result.bSuccess)
    {
      // Save the new path in LayoutStateService
      LayoutStateService::getService()->updateLibraryPath(result.filePath);
      currentDirectory = ofDirectory(result.filePath);
      refresh();
    }
  }
  ImGui::SameLine();

  if (ImGui::Button("Show in Finder"))
  {
#if defined(_WIN32)
    // Windows
    std::string command = "explorer " + currentDirectory.getAbsolutePath();
    system(command.c_str());
#elif defined(__APPLE__)
    // macOS
    std::string command = "open " + currentDirectory.getAbsolutePath();
    system(command.c_str());
#else
    ofLogError("ofApp::openFolder") << "This function is not implemented for this platform.";
#endif
  }

  ImGui::SameLine();

  if (ImGui::Button("Refresh"))
  {
    refresh();
  }

  listBrowserView.draw();
}
