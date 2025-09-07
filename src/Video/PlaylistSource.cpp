#include "PlaylistSource.hpp"

void PlaylistSource::setup() {
  scanFolder();
  loadSelectedVideo();
  FileSource::setup();
}

void PlaylistSource::scanFolder() {
  auto settings = playlistSettings();
  settings->videoFiles.clear();
  
  ofDirectory dir(settings->folderPath);
  dir.allowExt("mp4");
  dir.allowExt("mov");
  dir.allowExt("avi");
  dir.allowExt("mkv");
  dir.allowExt("flv");
  dir.allowExt("webm");
  dir.listDir();
  
  for(int i = 0; i < dir.size(); i++) {
    ofFile file = dir.getFile(i);
    if(isVideoFile(file.getAbsolutePath())) {
      settings->videoFiles.push_back(file);
    }
  }
}

void PlaylistSource::loadSelectedVideo() {
  auto settings = playlistSettings();
  if(settings->videoFiles.empty()) return;
  
  if(settings->selectedVideoIndex >= settings->videoFiles.size()) {
    settings->selectedVideoIndex = 0;
  }
  
  path = settings->videoFiles[settings->selectedVideoIndex].getAbsolutePath();
}

void PlaylistSource::drawSettings() {
  auto settings = playlistSettings();
  
  if(ImGui::Button("Select Folder")) {
    ofFileDialogResult result = ofSystemLoadDialog("Select Videos Folder", true); // true for folder selection
    if(result.bSuccess) {
      settings->folderPath = result.getPath();
      scanFolder();
      loadSelectedVideo();
      setup();
    }
  }
  
  ImGui::SameLine();
  ImGui::Text("%s", settings->folderPath.empty() ? "No folder selected" : settings->folderPath.c_str());
  
  if(settings->videoFiles.empty()) {
    ImGui::Text("No video files found in folder");
    return;
  }
  
  ImGui::Text("Videos:");
  for(int i = 0; i < settings->videoFiles.size(); i++) {
    bool isSelected = (i == settings->selectedVideoIndex);
    if(ImGui::Selectable(settings->videoFiles[i].getFileName().c_str(), isSelected)) {
      settings->selectedVideoIndex = i;
      loadSelectedVideo();
      setup();
    }
  }
  
  FileSource::drawSettings();
}
