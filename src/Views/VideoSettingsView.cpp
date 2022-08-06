//
//  VideoFeedSettingsView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#include "VideoSettingsView.hpp"
#include "ofxImGui.h"
#include "BasicSettingsView.hpp"
#include "FontService.hpp"

const static ofVec2f windowSize = ofVec2f(800, 600);

static int streamIdCounter = 0;

void VideoSettingsView::setup() {
  basicSettingsView.setup();
  basicSettingsView.basicSettings = &videoSettings->basicSettings;
  feedback0SettingsView.feedbackSettings = &videoSettings->feedback0Settings;
  feedback1SettingsView.feedbackSettings = &videoSettings->feedback1Settings;
  feedback2SettingsView.feedbackSettings = &videoSettings->feedback2Settings;
  videoSettings->streamId = streamIdCounter += 1;
  styleWindow();
}

void VideoSettingsView::styleWindow() {
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  
  style.WindowPadding = ImVec2(20, 20);
  style.FramePadding = ImVec2(14, 2);
  style.ItemSpacing = ImVec2(8, 3);
  style.ItemInnerSpacing = ImVec2(2, 4);
  style.WindowRounding = 12.f;
}

void VideoSettingsView::update() {
  basicSettingsView.update();
}

void VideoSettingsView::draw() {
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;
  ImGui::SetNextWindowSize(windowSize);
  auto name = std::string("##%d Video Feed Tabs", videoSettings->streamId);
  
  ImGui::Begin(name.c_str(), NULL, windowFlags);
  ImGui::PushFont(FontService::getService()->h2);
  ImGui::Text("Video Feed Settings - %d", videoSettings->streamId);
  ImGui::PopFont();
  ImGui::Separator();
  
  if (ImGui::BeginTabBar(name.c_str(), ImGuiTabBarFlags_None))
  {
    if (ImGui::BeginTabItem("Basic Settings"))
    {
      basicSettingsView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 1"))
    {
      feedback0SettingsView.draw();
      if (ImGui::Button("Clear Feedback")) {
        videoStream->shouldClearFrameBuffer = true;
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 2"))
    {
      feedback1SettingsView.draw();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Feedback 3"))
    {
      feedback2SettingsView.draw();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::Separator();
  if (ImGui::Button("Close Stream")) {
    closeStream(videoSettings->streamId);
  }
  
  ImGui::End();
}

