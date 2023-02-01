//
//  FileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "FileBrowserView.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include "AvailableVideoSource.hpp"

void FileBrowserView::refresh() {
  files.clear();
  ofDirectory dir;
  dir.listDir(currentDirectory);
  dir.sort();
  for (int i = 0; i < dir.size(); i++) {
    bool isDirectory = dir.getFile(i).isDirectory();
    files.push_back(File(dir.getPath(i), isDirectory));
  }
  currentDirectory = dir.getAbsolutePath();
  std::vector<TileItem> tileItems = {};

  for (auto file : files) {
    // Open the file and get the first frame
    auto availableSource = std::make_shared<AvailableVideoSource>(file.name, VideoSource_file, ShaderSource_empty, 0, file.path);
    sources.push_back(availableSource);
    
    // Create a closure which will be called when the tile is clicked
    std::function<void()> dragCallback = [availableSource]() {
      // Create a payload to carry the video source
      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload("VideoSource", availableSource.get(), sizeof(AvailableVideoSource));
        ImGui::Text("%s", availableSource->sourceName.c_str());
        ImGui::EndDragDropSource();
      }
    };

    TileItem tileItem = TileItem(file.name, 0, 0, dragCallback);

    tileItems.push_back(tileItem);
  }
  tileBrowserView.tileItems = tileItems;
}

void FileBrowserView::setup() { refresh(); }

// Get a list of all files in the current directory.
// Create a File for each file and add it to the files vector.
void FileBrowserView::update() {}

// Display the Files in the files vector as items in a ListBox.
// If a file is selected, update settings.selectedFile
void FileBrowserView::draw() {
  // Add a button to change the current directory to the parent directory.
  if (ImGui::Button("Go Up")) {
    ofDirectory dir;
    dir.open(currentDirectory);
    // Get the parent directory of the current directory by removing the last
    // directory from the path.
    currentDirectory = dir.getAbsolutePath().substr(
        0, dir.getAbsolutePath().find_last_of("/\\"));
    refresh();
  }
  ImGui::SameLine();

  // Add a button to open the file browser to choose the current directory
  if (ImGui::Button("Choose Directory")) {
    ofFileDialogResult result = ofSystemLoadDialog("Choose Directory", true);
    if (result.bSuccess) {
      currentDirectory = result.filePath;
      refresh();
    }
  }
  
  tileBrowserView.draw();
}
