#include "MainApp.h"
#include "AudioSourceService.hpp"
#include "BookmarkService.hpp"
#include "MSABPMTapper.h"
#include "FileSource.hpp"
#include "FontService.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ConfigService.hpp"
#include "StrandService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "TransformShader.hpp"
#include "UUID.hpp"
#include "LayoutStateService.hpp"
#include "VideoSourceService.hpp"
#include "MarkdownService.hpp"
#include "WebcamSource.hpp"
#include "LibraryService.hpp"
#include "TextureService.hpp"
#include "functional"
#include "implot.h"
#include "ofMain.h"
#include "ofxImGui.h"
#include "Strings.hpp"
#include <stdio.h>

void MainApp::setup()
{
  glEnable(GL_DEPTH_TEST);
  if (isSetup)
    return;
  ofDisableArbTex();
  ofEnableAlphaBlending();
  ofDisableDepthTest();
  ModulationService::getService();
  MidiService::getService();
  AudioSourceService::getService();
  TextureService::getService();
  BookmarkService::getService();
  StrandService::getService();
  LayoutStateService::getService();
  gui.setup();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  FontService::getService()->addFontToGui(&gui);
  VideoSourceService::getService();
  MarkdownService::getService();
  ConfigService::getService()->loadDefaultConfigFile();
  StrandService::getService()->setup();
  mainStageView->setup();
  LibraryService::getService()->backgroundFetchLibraryFiles();
  ParameterService::getService()->setup();
  StringManager::loadStrings();
}

void MainApp::update()
{
  runMainThreadTasks();
  OscillationService::getService()->tickOscillators();
  ModulationService::getService()->tickMappings();
  ParameterService::getService()->tickParameters();
  VideoSourceService::getService()->updateVideoSources();
  ShaderChainerService::getService()->processFrame();
  ConfigService::getService()->checkAndSaveDefaultConfigFile();
  AudioSourceService::getService()->update();
  mainStageView->update();
}

void MainApp::draw()
{
  gui.begin();
  ImGui::PushFont(FontService::getService()->p);
  drawMainStage();
  ImGui::PopFont();
  gui.end();
}

void MainApp::drawMainStage()
{
  // Get the current screen size
  ImGui::SetNextWindowSize(getScaledWindowSize());
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoScrollbar;
  
  ImGui::Begin("Main Stage", NULL, windowFlags);
  ImGui::PushFont(FontService::getService()->p);
  mainStageView->draw();
  ImGui::PopFont();
  ImGui::End();
}

void MainApp::exitStream(ofEventArgs &args) {}

void MainApp::dragEvent(ofDragInfo dragInfo)
{
  // Check if the file is a JSON file
  if (ofIsStringInString(dragInfo.files[0], ".json"))
  {
    ConfigService::getService()->loadStrandFile(dragInfo.files[0]);
    return;
  }
  
  // Check if the file is a video file
  if (ofIsStringInString(dragInfo.files[0], ".mp4") ||
      ofIsStringInString(dragInfo.files[0], ".mov") ||
      ofIsStringInString(dragInfo.files[0], ".avi") ||
      ofIsStringInString(dragInfo.files[0], ".mkv") ||
      ofIsStringInString(dragInfo.files[0], ".flv") ||
      ofIsStringInString(dragInfo.files[0], ".wmv"))
  {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dragInfo.files[0]);
    
    auto videoSource = VideoSourceService::getService()->addFileVideoSource(fileName, dragInfo.files[0]);
    NodeLayoutView::getInstance()->fileDropInProgress = true;
    NodeLayoutView::getInstance()->handleDroppedSource(videoSource);
    return;
  }
  
  // Check if the file is a video file
  if (ofIsStringInString(dragInfo.files[0], ".png") ||
      ofIsStringInString(dragInfo.files[0], ".jpg") ||
      ofIsStringInString(dragInfo.files[0], ".jpeg") ||
      ofIsStringInString(dragInfo.files[0], ".gif"))
  {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dragInfo.files[0]);
    auto videoSource = VideoSourceService::getService()->addImageVideoSource(fileName, dragInfo.files[0]);
    NodeLayoutView::getInstance()->handleDroppedSource(videoSource);
  }
}

void MainApp::executeOnMainThread(const std::function<void()>& task) {
  std::lock_guard<std::mutex> lock(mainThreadTasksMutex);
  mainThreadTasks.push(task);
}

void MainApp::runMainThreadTasks() {
  std::queue<std::function<void()>> tasksToRun;
  
  {
    std::lock_guard<std::mutex> lock(mainThreadTasksMutex);
    std::swap(tasksToRun, mainThreadTasks);
  }
  
  while (!tasksToRun.empty()) {
    auto task = tasksToRun.front();
    tasksToRun.pop();
    task();
  }
}

void MainApp::resetState() {}

void MainApp::keyReleased(int key) {
  mainStageView->keyReleased(key);
}
