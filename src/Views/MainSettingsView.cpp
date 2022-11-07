//
//  MainSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-15.
//

#include "MainSettingsView.hpp"
#include "ofxImGui.h"
#include "FontService.hpp"
#include "ofMain.h"

const static ofVec2f windowSize = ofVec2f(500, 500);

void MainSettingsView::setup() {
  auto ofAudioDevices = ofSoundStream().getDeviceList();
  auto ofVideoDevices = ofVideoGrabber().listDevices();
  
  for (int i = 0; i < ofVideoDevices.size(); i++) {
    videoDeviceNames.push_back(std::string(ofVideoDevices[i].deviceName));
  }
  
  for (int i = 0; i < ofAudioDevices.size(); i++) {
    audioDeviceNames.push_back(std::string(ofAudioDevices[i].name));
  }
    
  if (videoDeviceNames.empty()) {
    std::string str = "No Source Found";
    videoDeviceNames.push_back(str);
  }
  
  if (audioDeviceNames.empty()) {
    std::string str = "No Source Found";
    audioDeviceNames.push_back(str);
  }
}

void MainSettingsView::update(){
  
}

void MainSettingsView::draw() {
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;
  ImGui::SetNextWindowSize(windowSize);
  bool addTapped = false;
  std::string name = "New Stream";
  if (ImGui::Begin("main_settings_view", NULL, windowFlags)) {
    ImGui::PushFont(FontService::getService()->h2);
    ImGui::Text("Main Settings");
    ImGui::PopFont();
    ImGui::Separator();

    drawVideoSettings();
    
    // Add Stream button (if settings are valid)
    
    if (mainSettings->canLaunchVideo()) {
      if (ImGui::Button("Add Video Stream")) {
        addTapped = true;
      }
    }
    
    drawAudioSettings();
    if (mainSettings->canLaunchAudio()) {
      if (ImGui::Button("Add Audio Stream")) {
        addAudioStream(AudioStreamConfig(mainSettings->selectedAudioSource, mainSettings->selectedAudioPath, mainSettings->selectedAudioDeviceIndex));
      }
    }

    ImGui::PushFont(FontService::getService()->p);
    ImGui::Text("%d active streams", mainSettings->activeStreams);
    ImGui::PopFont();
  }
  if (addTapped) {
    addVideoStream(StreamConfig(mainSettings->selectedVideoSource,
                                mainSettings->selectedVideoPath,
                                name,
                                mainSettings->selectedVideoDeviceIndex));
  }
  ImGui::End();
}

// Drawing

void MainSettingsView::drawVideoSettings() {
  std::string sourceName;
  switch (mainSettings->selectedVideoSource) {
    case VideoSource_file:
      sourceName = "Video File";
      break;
    case VideoSource_webcam:
      sourceName = "Webcam";
  }
  
  if (ImGui::BeginCombo(sourceName.c_str(), "Select Source"))
  {
    if (ImGui::Selectable("Webcam", mainSettings->selectedVideoSource == VideoSource_webcam))
      mainSettings->selectedVideoSource = VideoSource_webcam;
    if (ImGui::Selectable("Video File", mainSettings->selectedVideoSource == VideoSource_file))
      mainSettings->selectedVideoSource = VideoSource_file;
    ImGui::EndCombo();
  }
  
  CommonViews::sSpacing();
  
  // Webcam Select
  
  if (mainSettings->selectedVideoSource == VideoSource_webcam) {
    if (ImGui::BeginCombo(videoDeviceNames[mainSettings->selectedVideoDeviceIndex].c_str(), "Select Webcam"))
    {
      for (int i = 0; i < videoDeviceNames.size(); i++) {
        auto name = videoDeviceNames[i];
        // Option for every device
        if (ImGui::Selectable(name.c_str(), mainSettings->selectedVideoDeviceIndex == i)) {
            mainSettings->selectedVideoDeviceIndex = i;
            mainSettings->selectedVideoDevice = name;
        }
          
      }
      ImGui::EndCombo();
    }
  }
  
  // File picker
  
  if (mainSettings->selectedVideoSource == VideoSource_file) {
    if (ImGui::Button("Choose File")) {
      ofFileDialogResult result = ofSystemLoadDialog("Load video file");
      
      if(result.bSuccess) {
        mainSettings->selectedVideoPath = result.getPath();
      }
    }
    CommonViews::sSpacing();
    auto string = mainSettings->selectedVideoPath.empty() ? "No file selected" : mainSettings->selectedVideoPath;
    ImGui::Text("%s", string.c_str());
  }
}

void MainSettingsView::drawAudioSettings() {
  // Audio Stream
  std::string sourceName;
  switch (mainSettings->selectedAudioSource) {
    case AudioSource_none:
      sourceName = "None";
    case AudioSource_microphone:
      sourceName = "Microphone";
    case AudioSource_file:
      sourceName = "Audio File";
  }
  
  if (ImGui::BeginCombo(sourceName.c_str(), "(Optional) Select Audio Source"))
  {
    if (ImGui::Selectable("None", mainSettings->selectedAudioSource == AudioSource_none))
      mainSettings->selectedAudioSource = AudioSource_none;
    if (ImGui::Selectable("Microphone", mainSettings->selectedAudioSource == AudioSource_microphone))
      mainSettings->selectedAudioSource = AudioSource_microphone;
    if (ImGui::Selectable("Audio File", mainSettings->selectedAudioSource == AudioSource_file))
      mainSettings->selectedAudioSource = AudioSource_file;
    ImGui::EndCombo();
  }
  
  if (mainSettings->selectedAudioSource == AudioSource_microphone) {
    if (ImGui::BeginCombo(audioDeviceNames[mainSettings->selectedAudioDeviceIndex].c_str(), "Select Recording Device"))
    {
      for (int i = 0; i < audioDeviceNames.size(); i++) {
        auto name = audioDeviceNames[i];
        // Option for every device
        if (ImGui::Selectable(name.c_str(), mainSettings->selectedAudioDeviceIndex == i))
          mainSettings->selectedAudioDeviceIndex = i;
      }
      ImGui::EndCombo();
    }
  }
}


