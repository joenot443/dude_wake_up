#define JSON_DIAGNOSTICS 1
#define IMGUI_DEFINE_MATH_OPERATORS 1

#include <errno.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

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

#ifdef _WIN32
#include <windows.h>
#include "Utf8Path.hpp"
#endif


const static ofVec2f windowSize = ofVec2f(2400, 1600);

using json = nlohmann::json;

// Static member definitions moved to src/Engine/ServiceStatics.cpp
// so they can be shared between dude_wake_up and NottawaApp targets.

void setupDirectories() {
#if defined(TARGET_WIN32)
  // Windows: data ships as bin/data next to the executable. Build step
  // (packaging/windows/build_release.ps1 + vcxproj post-build) robocopies
  // it there. Empty string leaves OF's default "bin/data" resolution.
  ofSetDataPathRoot("data/");
#else
  ofSetDataPathRoot("../Resources/data/");
#endif

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

// Try to create the OF window at the given GL version. Returns a shared_ptr
// (empty on failure). ofCreateWindow can succeed with a partially usable
// window on some drivers, so we probe glfwGetCurrentContext() after.
static std::shared_ptr<ofAppBaseWindow> tryCreateWindow(int major, int minor) {
  ofGLFWWindowSettings settings;
  settings.setGLVersion(major, minor);
  settings.setSize(1440, 900);
  auto window = ofCreateWindow(settings);
  if (!window) return nullptr;

  auto* glfwWindow = dynamic_cast<ofAppGLFWWindow*>(window.get());
  if (!glfwWindow || !glfwWindow->getGLFWWindow()) {
    ofLogError("main") << "GL " << major << "." << minor
                      << " context creation failed (no GLFW window)";
    return nullptr;
  }
  ofLogNotice("main") << "GL " << major << "." << minor << " context created";
  return window;
}

#ifdef _WIN32
// Dev-only: mirror ofLog output to a file next to the exe. Nottawa.exe
// uses SubSystem=Windows so stdout/stderr go nowhere by default; without
// this a boot crash leaves no trail. Opt in via NOTTAWA_BOOT_LOG=1 at
// launch — defaults to off so release users don't get a mystery log.
static void installFileLogger_() {
  const char* envVal = std::getenv("NOTTAWA_BOOT_LOG");
  if (!envVal || envVal[0] != '1') return;
  static class FileLogger : public ofBaseLoggerChannel {
  public:
    FileLogger() {
      f_ = std::fopen("nottawa-boot.log", "wb");
      if (f_) { std::fputs("--- boot ---\n", f_); std::fflush(f_); }
    }
    void log(ofLogLevel level, const std::string& module, const std::string& message) override {
      if (!f_) return;
      std::fprintf(f_, "[%d] %s: %s\n", (int)level, module.c_str(), message.c_str());
      std::fflush(f_);
    }
    FILE* f_ = nullptr;
  } logger;
  ofSetLoggerChannel(std::shared_ptr<ofBaseLoggerChannel>(&logger, [](ofBaseLoggerChannel*){}));
}
#endif

int main( ){
#ifdef _WIN32
  installFileLogger_();
#endif
  setupDirectories();
  auto homeDir = ofFilePath::getUserHomeDir();
  auto libraryPath = ofFilePath::join(homeDir, "/nottawa");
  auto sentryPath = ofFilePath::join(homeDir, "/nottawa/sentry");

  // Windows GPUs range from modern discrete cards down to Sandy/Ivy Bridge
  // iGPUs. Ask for 3.2 first (parity with macOS Core Profile), then step
  // down. If nothing works, tell the user the machine is unsupported
  // rather than crashing on a null context.
  std::shared_ptr<ofAppBaseWindow> window = tryCreateWindow(3, 2);
  if (!window) window = tryCreateWindow(3, 1);
  if (!window) window = tryCreateWindow(3, 0);
  if (!window) {
#ifdef _WIN32
    const wchar_t* msg =
      L"Nottawa requires OpenGL 3.0 or newer.\n\n"
      L"Your GPU driver reports a lower version — this often means "
      L"Microsoft's software renderer is being used instead of your real GPU.\n\n"
      L"Install the latest vendor driver from Intel, NVIDIA, or AMD, then try again.";
    MessageBoxW(nullptr, msg, L"Nottawa — unsupported GPU", MB_OK | MB_ICONERROR);
#else
    ofLogFatalError("main") << "Could not create any GL 3.x context. Aborting.";
#endif
    return 1;
  }

  auto glfwWindow = dynamic_cast<ofAppGLFWWindow*>(window.get())->getGLFWWindow();
#ifdef _WIN32
  // Daandelange's ofxImGui EngineGLFW replaces OF's event listeners; the
  // window stays hidden+0x0 until explicitly shown and sized. Set a usable
  // size, position centered-ish, then show + maximize.
  glfwSetWindowSize(glfwWindow, 1440, 900);
  glfwSetWindowPos(glfwWindow, 100, 100);
  glfwShowWindow(glfwWindow);
#endif
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



