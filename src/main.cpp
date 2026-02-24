#define JSON_DIAGNOSTICS 1
#define IMGUI_DEFINE_MATH_OPERATORS 1

#include <errno.h>
#include <CoreFoundation/CoreFoundation.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "CommonViews.hpp"
#include "ofMain.h"
#include "GLFW/glfw3.h"
#include "MainApp.h"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "AudioSourceService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "ImageService.hpp"
#include "FeedbackSourceService.hpp"
#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "MarkdownService.hpp"
#include "TextureService.hpp"
#include "ShaderChainerService.hpp"
#include "LayoutStateService.hpp"
#include "HelpService.hpp"
#include "BookmarkService.hpp"
#include "StrandService.hpp"
#include "TimeService.hpp"
#include "ParameterService.hpp"
#include "IconService.hpp"
#include "URLSchemeHandler.h"


const static ofVec2f windowSize = ofVec2f(2400, 1600);

using json = nlohmann::json;

// Static member definitions moved to src/Engine/ServiceStatics.cpp
// so they can be shared between dude_wake_up and NottawaApp targets.

void setupDirectories() {
  ofSetDataPathRoot("../Resources/data/");
  
  auto appSupportPath = ConfigService::appSupportFilePath();
  auto libraryPath = appSupportPath + "/nottawa";
  auto sentryPath = libraryPath + "/sentry";
  auto shadersPath = libraryPath + "/shaders";
  auto videosPath = libraryPath + "/videos";
  auto strandsPath = libraryPath + "/strands";
  auto exportPath = libraryPath + "/exports";
  
  if (!ofDirectory::doesDirectoryExist(libraryPath)) {
    ofDirectory::createDirectory(libraryPath);
  }
  if (!ofDirectory::doesDirectoryExist(sentryPath)) {
    ofDirectory::createDirectory(sentryPath);
  }
  if (!ofDirectory::doesDirectoryExist(shadersPath)) {
    ofDirectory::createDirectory(shadersPath);
  }
  if (!ofDirectory::doesDirectoryExist(videosPath)) {
    ofDirectory::createDirectory(videosPath);
  }
  if (!ofDirectory::doesDirectoryExist(strandsPath)) {
    ofDirectory::createDirectory(strandsPath);
  }
  if (!ofDirectory::doesDirectoryExist(exportPath)) {
    ofDirectory::createDirectory(exportPath);
  }
  
  for (auto dir : ConfigService::getService()->shaderConfigFoldersPaths()) {
    if (!ofDirectory::doesDirectoryExist(dir)) {
      ofDirectory::createDirectory(dir);
    }
  }
}

int main( ){
  setupDirectories();
  auto homeDir = ofFilePath::getUserHomeDir();
  auto libraryPath = ofFilePath::join(homeDir, "/nottawa");
  auto sentryPath = ofFilePath::join(homeDir, "/nottawa/sentry");
  
  ofGLFWWindowSettings settings;
  settings.setGLVersion(3, 2);
  settings.setSize(1440, 900);
  auto window = ofCreateWindow(settings);
  // Maximize the window to fill the screen
  auto glfwWindow = dynamic_cast<ofAppGLFWWindow*>(window.get())->getGLFWWindow();
  glfwMaximizeWindow(glfwWindow);
  auto app = shared_ptr<MainApp>(new MainApp(window));
  ofSetFrameRate(60);
  ofEnableAntiAliasing();
  ofEnableSmoothing();
  ofSetEscapeQuitsApp(false);
  ofSetWindowTitle("Nottawa");
  ofSetLogLevel(OF_LOG_NOTICE);

  // Initialize URL scheme handler for nottawa:// links
  #ifdef TARGET_OSX
  URLSchemeHandler::getInstance().initialize();
  #endif

  ofRunApp(window, app);
  ofRunMainLoop();
  return 0;
}



