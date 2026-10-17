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
#include "URLSchemeHandler.h"

static void logGLEnvironmentOnce() {
  static bool logged = false;
  if (logged) return;
  logged = true;

  const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
  const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const char* glsl     = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
  ofLogNotice("GL") << "vendor="   << (vendor   ? vendor   : "(null)");
  ofLogNotice("GL") << "renderer=" << (renderer ? renderer : "(null)");
  ofLogNotice("GL") << "version="  << (version  ? version  : "(null)");
  ofLogNotice("GL") << "glsl="     << (glsl     ? glsl     : "(null)");

  GLint format = 0, type = 0;
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
  glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
  ofLogNotice("GL") << "impl_color_read_format=0x" << std::hex << format
                    << " type=0x" << type << std::dec;

  GLboolean srgb = GL_FALSE;
  glGetBooleanv(GL_FRAMEBUFFER_SRGB, &srgb);
  ofLogNotice("GL") << "framebuffer_srgb=" << (srgb ? "ENABLED" : "disabled");
}

void MainApp::setup()
{
  glEnable(GL_DEPTH_TEST);
  if (isSetup)
    return;
  ofDisableArbTex();
  ofEnableAlphaBlending();
  ofDisableDepthTest();
  logGLEnvironmentOnce();
  ModulationService::getService();
  AudioSourceService::getService();
  TextureService::getService();
  BookmarkService::getService();
  StrandService::getService();
  LayoutStateService::getService();
#ifdef _WIN32
  // Daandelange's ofxImGui default autoDraw=true isn't painting on Windows
  // (listener on window->events().draw at OF_EVENT_ORDER_AFTER_APP doesn't
  // fire for us). Switch to manual draw mode: gui.end() closes the frame,
  // we explicitly call gui.draw() after to render.
  gui.setup(nullptr, false);
#else
  gui.setup();
#endif
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
  affirmScale();
  MidiService::getService();
  StringManager::loadStrings();

  // Process any pending URL that launched the app
  #ifdef TARGET_OSX
  URLSchemeHandler::getInstance().processPendingURL();
  #endif
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
  MidiService::getService()->update();
  mainStageView->update();
  affirmWindowSize();
  affirmScale();
}

void MainApp::draw()
{
  gui.begin();
  drawMainStage();
  gui.end();
#ifdef _WIN32
  // Manual draw since Windows build opts out of autoDraw in setup().
  gui.draw();
#endif
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
  // Guard against FontService::current being null or its `p` not yet
  // loaded (happens briefly during first draw on Windows). Fall back to
  // the current ImGui default font — the UI will look wrong until the
  // real font arrives but it won't crash.
  auto* fs = FontService::getService();
  ImFont* font = (fs && fs->current) ? fs->current->p : nullptr;
  if (font) ImGui::PushFont(font);
  mainStageView->draw();
  if (font) ImGui::PopFont();
  ImGui::End();
  CommonViews::PopRedesignStyle();
}

void MainApp::exitStream(ofEventArgs &args) {}

void MainApp::dragEvent(ofDragInfo dragInfo)
{
  // OF 0.12 returns std::filesystem::path from dragInfo.files; MSVC (unlike
  // Apple libc++) refuses the implicit conversion to std::string. Materialize
  // once at the top so the rest of the function keeps its old shape.
  const std::string dropped = dragInfo.files[0].string();

  if (ofFile(dropped).isDirectory()) {
    BookmarkService::getService()->saveBookmarkForDirectory(dropped);
    BookmarkService::getService()->beginAccessingBookmark(dropped);
    mainStageView->loadDirectory(dropped);
    return;
  }

  // Check if the file is a JSON file
  if (ofIsStringInString(dropped, ".json"))
  {
    ConfigService::getService()->loadStrandFile(dropped);
    return;
  }

  // Check if the file is a video file
  if (ofIsStringInString(dropped, ".mp4") ||
      ofIsStringInString(dropped, ".mov") ||
      ofIsStringInString(dropped, ".avi") ||
      ofIsStringInString(dropped, ".mkv") ||
      ofIsStringInString(dropped, ".flv") ||
      ofIsStringInString(dropped, ".wmv"))
  {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dropped);

    auto videoSource = ActionService::getService()->addFileVideoSource(fileName, dropped);
    NodeLayoutView::getInstance()->fileDropInProgress = true;
    NodeLayoutView::getInstance()->handleDroppedSource(videoSource);
    return;
  }

  // Check if the file is a video file
  if (ofIsStringInString(dropped, ".png") ||
      ofIsStringInString(dropped, ".jpg") ||
      ofIsStringInString(dropped, ".jpeg") ||
      ofIsStringInString(dropped, ".gif"))
  {
    // Create a new VideoSource for the file
    auto fileName = ofFilePath::getFileName(dropped);
    auto videoSource = ActionService::getService()->addImageVideoSource(fileName, dropped);
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
