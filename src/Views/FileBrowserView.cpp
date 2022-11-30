//
//  FileBrowserView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#include "FileBrowserView.hpp"
#include "ofxImGui.h"
#include "ofMain.h"


void FileBrowserSettings::refresh() {
  files.clear();
  ofDirectory dir;
  dir.listDir(currentDirectory);
  dir.sort();
  for (int i = 0; i < dir.size(); i++) {
    bool isDirectory = dir.getFile(i).isDirectory();
    files.push_back(File(dir.getPath(i), isDirectory));
  }
  currentDirectory = dir.getAbsolutePath();
}

void FileBrowserView::setup() {
  settings->refresh();
}

// Get a list of all files in the current directory.
// Create a File for each file and add it to the files vector.
void FileBrowserView::update() {

}


// Display the Files in the files vector as items in a ListBox.
// If a file is selected, update settings.selectedFile
void FileBrowserView::draw() {
  if (ImGui::BeginListBox(settings->currentDirectory.c_str())) {
  for (int i = 0; i < settings->files.size(); i++) {
    if (ImGui::Selectable(settings->files[i].name.c_str())) {
      if (settings->files[i].isDirectory) {
        settings->currentDirectory = settings->files[i].path;
      } else {
        settings->selectedFile = std::make_shared<File>(settings->files[i]);
      }
    }
  }
    ImGui::EndListBox();
  }

  // Add a button to change the current directory to the parent directory.
  if (ImGui::Button("Go Up")) {
    ofDirectory dir;
    dir.open(settings->currentDirectory);
    // Get the parent directory of the current directory by removing the last
    // directory from the path.
    settings->currentDirectory = dir.getAbsolutePath().substr(0, dir.getAbsolutePath().find_last_of("/\\"));
    settings->refresh();
  }
  ImGui::SameLine();

  // Add a button to open the file browser to choose the current directory
  if (ImGui::Button("Choose Directory")) {
    ofFileDialogResult result = ofSystemLoadDialog("Choose Directory", true);
    if (result.bSuccess) {
      settings->currentDirectory = result.getPath();
      settings->refresh();
    }
  }
}
