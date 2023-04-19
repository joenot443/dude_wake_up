#include "MainApp.h"
#include "AudioSourceService.hpp"
#include "FileSource.hpp"
#include "FontService.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ConfigService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainer.hpp"
#include "ShaderChainerService.hpp"
#include "TransformShader.hpp"
#include "UUID.hpp"
#include "LayoutStateService.hpp"
#include "VideoSourceService.hpp"
#include "MarkdownService.hpp"
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
  ofEnableAlphaBlending();
  ofDisableDepthTest();
  ModulationService::getService();
  MidiService::getService();
  AudioSourceService::getService();
  LayoutStateService::getService();
  gui.setup();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  FontService::getService()->addFontToGui(&gui);
  VideoSourceService::getService();
  MarkdownService::getService();
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
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoBringToFrontOnFocus;
  
  ImGui::Begin("Main Stage", NULL, windowFlags);
  ImGui::PushFont(FontService::getService()->p);
  mainStageView->draw();
  ImGui::PopFont();
  ImGui::End();
}

void MainApp::exitStream(ofEventArgs &args) {}

void MainApp::dragEvent(ofDragInfo dragInfo) {
  // Check if the file is a JSON file
  if (ofIsStringInString(dragInfo.files[0], ".json")) {
    ConfigService::getService()->loadShaderChainerFile(dragInfo.files[0]);
    return;
  }
  
  // Check if the file is a video file
  if (ofIsStringInString(dragInfo.files[0], ".mp4") ||
      ofIsStringInString(dragInfo.files[0], ".mov") ||
      ofIsStringInString(dragInfo.files[0], ".avi") ||
      ofIsStringInString(dragInfo.files[0], ".mkv") ||
      ofIsStringInString(dragInfo.files[0], ".flv") ||
      ofIsStringInString(dragInfo.files[0], ".wmv")){
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dragInfo.files[0]);
    
    auto videoSource = std::make_shared<FileSource>(
                                                    UUID::generateUUID(), fileName, dragInfo.files[0]);
    VideoSourceService::getService()->addVideoSource(videoSource, videoSource->id);
    ShaderChainerService::getService()->addNewShaderChainer(videoSource);
    mainStageView->nodeLayoutView.handleDroppedSource(videoSource);
    return;
  }
  
  // Check if the file is a video file
  if (ofIsStringInString(dragInfo.files[0], ".png") ||
      ofIsStringInString(dragInfo.files[0], ".jpg") ||
      ofIsStringInString(dragInfo.files[0], ".jpeg") ||
      ofIsStringInString(dragInfo.files[0], ".gif")) {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dragInfo.files[0]);
    auto videoSource = VideoSourceService::getService()->addImageVideoSource(fileName, dragInfo.files[0]);
    ShaderChainerService::getService()->addNewShaderChainer(videoSource);
    mainStageView->nodeLayoutView.handleDroppedSource(videoSource);
  }
}

void MainApp::resetState() {}

void MainApp::keyReleased(int key) { mainStageView->keyReleased(key); }
