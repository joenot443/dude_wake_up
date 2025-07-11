#define IMGUI_DEFINE_MATH_OPERATORS 1

#include "MainApp.h"
#include "AudioSourceService.hpp"
#include "ActionService.hpp"
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
#include "ImageService.hpp"
#include "WebcamSource.hpp"
#include "LibraryService.hpp"
#include "TextureService.hpp"
#include "TimeService.hpp"
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
  setupStyle(); // Apply the style
  FontService::getService()->setup();
  VideoSourceService::getService();
  MarkdownService::getService();
  ConfigService::getService()->loadDefaultConfigFile();
  StrandService::getService()->setup();
  ImageService::getService()->setup();
  mainStageView->setup();
  LibraryService::getService()->backgroundFetchLibraryFiles();
  LibraryService::getService()->backgroundFetchShaderCredits();
  ParameterService::getService()->setup();
  TimeService::getService()->setup();
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
  TimeService::getService()->tick();
  ConfigService::getService()->checkAndSaveDefaultConfigFile();
  AudioSourceService::getService()->update();
  mainStageView->update();
  affirmWindowSize();
  affirmScale();
}

void MainApp::draw()
{
  gui.begin();
  drawMainStage();
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
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoScrollWithMouse;
  
  CommonViews::PushRedesignStyle();
  ImGui::Begin("Main Stage", NULL, windowFlags);
  ImGui::PushFont(FontService::getService()->current->p);
  mainStageView->draw();
  ImGui::PopFont();
  ImGui::End();
  CommonViews::PopRedesignStyle();
}

void MainApp::exitStream(ofEventArgs &args) {}

void MainApp::dragEvent(ofDragInfo dragInfo)
{
  
  if (ofFile(dragInfo.files[0]).isDirectory()) {
    BookmarkService::getService()->saveBookmarkForDirectory(dragInfo.files[0]);
    BookmarkService::getService()->beginAccessingBookmark(dragInfo.files[0]);
    mainStageView->loadDirectory(dragInfo.files[0]);
    return;
  }
  
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
    
    auto videoSource = ActionService::getService()->addFileVideoSource(fileName, dragInfo.files[0]);
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
    auto videoSource = ActionService::getService()->addImageVideoSource(fileName, dragInfo.files[0]);
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

void MainApp::keyPressed(int key) {
  mainStageView->keyPressed(key);
}

void MainApp::keyReleased(int key) {
  mainStageView->keyReleased(key);
}

void MainApp::setupStyle()
{
  ImGuiStyle& imguiStyle = ImGui::GetStyle();
  imguiStyle.Alpha = style.alpha;
  imguiStyle.WindowPadding = ImVec2(style.windowPadding[0], style.windowPadding[1]);
  imguiStyle.FramePadding = ImVec2(style.framePadding[0], style.framePadding[1]);
  imguiStyle.CellPadding = ImVec2(style.framePadding[0], style.framePadding[1]);
}

void MainApp::affirmWindowSize() {
  if (ofGetWindowWidth() < 500) {
    ofSetWindowShape(500, ofGetWindowHeight());
  }
}

void MainApp::affirmScale() {
  auto windowPtr = dynamic_cast<ofAppGLFWWindow*>(ofGetWindowPtr());
  float newScale = windowPtr ? windowPtr->getPixelScreenCoordScale() : 1.0f;
  
  if (currentPixelScale == newScale) return;
  
  log("Switching font scale: %.1f → %.1f", currentPixelScale, newScale);
  currentPixelScale = newScale;
  FontService::getService()->useFontSetForScale(currentPixelScale);
  gui.setup();
  ImGui::GetStyle().ScaleAllSizes(currentPixelScale);
}
