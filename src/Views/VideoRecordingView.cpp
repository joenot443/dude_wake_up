//
//  VideoRecordingView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/25/24.
//

#include "VideoRecordingView.hpp"
#include "CommonViews.hpp"

void VideoRecordingView::setup() {
  
}

void VideoRecordingView::update() {
  
}

void VideoRecordingView::draw() {
  ImGui::Begin("Record Video");
  CommonViews::H3Title("File Location");
  ImGui::Text("%s", settings.filePath.c_str());
  if (ImGui::Button("Choose")) {
    auto result = ofSystemSaveDialog("Output.mp4", "Video Location");
    if (result.bSuccess) {
      settings.filePath = result.filePath;
    }
  }
  if (ImGui::BeginCombo("Output Windows", 0)) {
    
  }
  
  if (ImGui::Button("Record")) {
    
  }
  ImGui::End();
}


