//
//  NewVideoSourceView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/9/22.
//

#include "NewVideoSourceView.hpp"
#include "ofxImGui.h"
#include "VideoSourceService.hpp"

// Draw the new video source view.
// Allows the user to select a video source type and then create a new video source of that type.
// If the user selects the webcam type, they can choose which webcam to use from a dropdown.
// If the user selects the file type, draw a FileBrowserView. Use FileBrowserView.settings.selectedFile as the video source.
// When the user presses the Add button, create a new video source and add it to the service.

void NewVideoSourceView::draw() {
  if (ImGui::RadioButton("Webcam", settings.selectedType == VideoSource_webcam)) {
    settings.selectedType = VideoSource_webcam;
  }
  ImGui::SameLine();
  if (ImGui::RadioButton("File", settings.selectedType == VideoSource_file)) {
    settings.selectedType = VideoSource_file;
  }
  if (settings.selectedType == VideoSource_webcam) {
    std::vector<std::string> webcamNames = VideoSourceService::getService()->getWebcamNames();
    if (ImGui::BeginCombo("##WebcamSource", webcamNames[settings.selectedWebcamIndex].c_str())) {
      for (int i = 0; i < webcamNames.size(); i++) {
        if (ImGui::Selectable(webcamNames[i].c_str())) {
          settings.selectedWebcamIndex = i;
          settings.selectedWebcamName = webcamNames[i];
        }
      }
      ImGui::EndCombo();
    }
  } else if (settings.selectedType == VideoSource_file) {
    fileBrowserView.draw();
  }
  if (ImGui::Button("Add")) {
    if (settings.selectedType == VideoSource_webcam) {
      VideoSourceService::getService()->addWebcamVideoSource(settings.selectedWebcamName, settings.selectedWebcamIndex);
    } else if (settings.selectedType == VideoSource_file) {
      VideoSourceService::getService()->addFileVideoSource(fileBrowserView.settings->selectedFile->name, fileBrowserView.settings->selectedFile->path);
    }
  }
}


