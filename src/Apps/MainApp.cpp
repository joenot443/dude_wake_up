#include "MainApp.h"
#include "AudioSourceService.hpp"
#include "FileSource.hpp"
#include "FontService.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainer.hpp"
#include "ShaderChainerService.hpp"
#include "TransformShader.hpp"
#include "UUID.hpp"
#include "VideoSourceService.hpp"
#include "WebcamSource.hpp"
#include "functional"
#include "implot.h"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

const static ofVec2f windowSize = ofVec2f(1200, 800);

void MainApp::setup() {
  if (isSetup)
    return;
  ofDisableArbTex();
  ModulationService::getService();
  MidiService::getService();
  AudioSourceService::getService();
  gui.setup();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  FontService::getService()->addFontToGui(&gui);

  mainStageView->setup();
}

void MainApp::update() {
  OscillationService::getService()->tickOscillators();
  ModulationService::getService()->tickMappings();
  ParameterService::getService()->tickParameters();
  VideoSourceService::getService()->updateVideoSources();
  ShaderChainerService::getService()->updateShaderChainers();

  mainStageView->update();
}

void MainApp::draw() {
  gui.begin();
  ImGui::PushFont(FontService::getService()->p);
  drawMainStage();
  //  ImGui::ShowDemoWindow();
  ImGui::PopFont();
  gui.end();
}

void MainApp::drawMainStage() {
  // Get the current screen size
  ImGui::SetNextWindowSize(ImVec2(ofGetWindowWidth(), ofGetWindowHeight()));
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_MenuBar;
  ImGui::Begin("Main Stage", NULL, windowFlags);
  ImGui::PushFont(FontService::getService()->p);
  mainStageView->draw();
  ImGui::PopFont();
  ImGui::End();
}

void MainApp::exitStream(ofEventArgs &args) {}

void MainApp::dragEvent(ofDragInfo dragInfo) {
  // Check if the file is a video file
  if (ofIsStringInString(dragInfo.files[0], ".mp4") ||
      ofIsStringInString(dragInfo.files[0], ".mov") ||
      ofIsStringInString(dragInfo.files[0], ".avi") ||
      ofIsStringInString(dragInfo.files[0], ".mkv") ||
      ofIsStringInString(dragInfo.files[0], ".flv") ||
      ofIsStringInString(dragInfo.files[0], ".wmv") ||
      ofIsStringInString(dragInfo.files[0], ".webm") ||
      ofIsStringInString(dragInfo.files[0], ".gif")) {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dragInfo.files[0]);
    auto videoSource = std::make_shared<FileSource>(
        UUID::generateUUID(), fileName, dragInfo.files[0]);
    VideoSourceService::getService()->addVideoSource(videoSource);
  }
}

void MainApp::resetState() {}
