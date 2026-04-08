//
//  EngineLoop.cpp
//  dude_wake_up
//
//  Standalone engine update loop extracted from MainApp::update().
//

#include "EngineLoop.hpp"
#include "AudioSourceService.hpp"
#include "BookmarkService.hpp"
#include "ConfigService.hpp"
#include "FontService.hpp"
#include "ImageService.hpp"
#include "LayoutStateService.hpp"
#include "MidiService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"
#include "ShaderChainerService.hpp"
#include "StrandService.hpp"
#include "TextureService.hpp"
#include "TimeService.hpp"
#include "VideoSourceService.hpp"
#include "IOSurfaceTextureManager.hpp"
#include "nottawa_bridge.h"
#include "Strings.hpp"
#include "ofMain.h"
#include "GLFW/glfw3.h"
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>

// Hidden GLFW window for OpenGL context (NottawaApp only).
static GLFWwindow* hiddenWindow = nullptr;

EngineLoop* EngineLoop::instance = nullptr;

EngineLoop* EngineLoop::getInstance() {
  if (!instance) {
    instance = new EngineLoop();
  }
  return instance;
}

static void setupDataPath() {
  // Set OF data path from bundle Resources/data/ directory.
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if (mainBundle) {
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (resourcesURL) {
      char path[PATH_MAX];
      if (CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, PATH_MAX)) {
        std::string dataPath = std::string(path) + "/data/";
        ofSetDataPathRoot(dataPath);
      }
      CFRelease(resourcesURL);
    }
  }
}

/// Create a directory if it doesn't exist using POSIX stat/mkdir.
/// OF's ofDirectory::createDirectory fails silently on paths with spaces
/// (e.g. "Application Support"), so we use POSIX directly for NottawaApp.
static void ensureDirectory(const std::string& path) {
#ifdef NOTTAWA_ENGINE_ONLY
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    mkdir(path.c_str(), 0755);
  }
#else
  if (!ofDirectory::doesDirectoryExist(path)) {
    ofDirectory::createDirectory(path);
  }
#endif
}

static void setupDirectories() {
  auto appSupportPath = ConfigService::appSupportFilePath();
  auto libraryPath = appSupportPath + "/nottawa";

  std::vector<std::string> dirs = {
    libraryPath,
    libraryPath + "/sentry",
    libraryPath + "/shaders",
    libraryPath + "/videos",
    libraryPath + "/strands",
    libraryPath + "/exports"
  };

  for (auto& dir : dirs) {
    ensureDirectory(dir);
  }

  for (auto& dir : ConfigService::getService()->shaderConfigFoldersPaths()) {
    ensureDirectory(dir);
  }
}

static std::shared_ptr<ofAppBaseWindow> ofWindow = nullptr;

static bool setupOpenGLContext() {
  // Create a hidden OF window for full GL initialization.
  // OF needs its own window creation path to set up the programmable
  // renderer, extension detection, FBO support, etc.
  ofGLFWWindowSettings settings;
  settings.setGLVersion(3, 2);
  settings.setSize(64, 64);

  ofWindow = ofCreateWindow(settings);
  if (!ofWindow) {
    return false;
  }

  // Hide the GLFW window and disable its draw callback.
  // The GL context will be transferred to the engine thread, so the
  // GLFW refresh callback (triggered by macOS display layer on the main
  // thread) must not attempt GL calls or it will crash.
  auto glfwWindow = dynamic_cast<ofAppGLFWWindow*>(ofWindow.get());
  if (glfwWindow) {
    GLFWwindow* raw = glfwWindow->getGLFWWindow();
    glfwHideWindow(raw);
    glfwSetWindowRefreshCallback(raw, nullptr);
  }

  return true;
}

void EngineLoop::setup() {
  if (setupComplete) return;

  // Set up OF data path from bundle
  setupDataPath();

  // Create hidden OpenGL context if one doesn't already exist
  if (!glfwGetCurrentContext()) {
    if (!setupOpenGLContext()) {
      ofLogError("EngineLoop") << "Failed to create OpenGL context";
      return;
    }
  }

  // Force GL_TEXTURE_2D (not GL_TEXTURE_RECTANGLE_ARB) for FBOs.
  // Matches MainApp::setup() behavior.
  ofDisableArbTex();

  // Initialize services in dependency order.
  // This mirrors MainApp::setup() but omits all ImGui/UI initialization.
  ModulationService::getService();
  TextureService::getService();
  BookmarkService::getService();
  StrandService::getService();
  LayoutStateService::getService();

  VideoSourceService::getService();

  // Load font list (no ImGui dependency — just scans fonts/editor directory).
  // Must happen before config loading, which may create TextSource instances.
  FontService::getService()->loadFonts();

  // Create app support directories (needed by services during tick)
  setupDirectories();

  // Load saved config to restore last session
  ConfigService::getService()->loadDefaultConfigFile();

  StrandService::getService()->setup();

#ifndef NOTTAWA_ENGINE_ONLY
  ImageService::getService()->setup();
#endif

  ParameterService::getService()->setup();
  TimeService::getService()->setup();

  StringManager::loadStrings();

  setupComplete = true;
}

void EngineLoop::tick() {
  if (!setupComplete) return;

  // FPS tracking
  frameCount++;
  double now = ofGetElapsedTimef();
  if (now - lastFPSTime >= 1.0) {
    fps = static_cast<float>(frameCount) / static_cast<float>(now - lastFPSTime);
    frameCount = 0;
    lastFPSTime = now;
  }

  // 1. Run queued tasks
  runQueuedTasks();

  // 2. Update oscillators and modulation
  OscillationService::getService()->tickOscillators();
  ModulationService::getService()->tickMappings();

  // 3. Update parameters
  ParameterService::getService()->tickParameters();

  // 4. Update video sources (render to FBOs)
  VideoSourceService::getService()->updateVideoSources();

  // 5. Process shader chain (render effects)
  ShaderChainerService::getService()->processFrame();

  // 5b. Tick preview instances (browser live previews)
  ntw_tick_previews();

  // 5c. Blit tracked textures to IOSurfaces (no-op when nothing tracked)
  IOSurfaceTextureManager::getInstance()->blitTrackedTextures();

  // 6. Advance global time
  TimeService::getService()->tick();

  // 7. Auto-save config if needed
#ifdef NOTTAWA_ENGINE_ONLY
  // ofGetFrameNum() is always 0 in NottawaApp (no OF main loop), so
  // ConfigService::checkAndSaveDefaultConfigFile() never triggers.
  // Use time-based auto-save instead.
  if (now - lastAutoSaveTime >= autoSaveInterval && now >= 10.0) {
    lastAutoSaveTime = now;
    ConfigService::getService()->saveDefaultConfigFile();
  }
#else
  ConfigService::getService()->checkAndSaveDefaultConfigFile();
#endif

  // 8. Update audio analysis
  AudioSourceService::getService()->update();

  // 9. Update MIDI
  MidiService::getService()->update();
}

void EngineLoop::releaseContext() {
  glfwMakeContextCurrent(nullptr);
}

void EngineLoop::makeContextCurrent() {
  if (ofWindow) {
    auto glfwWindow = dynamic_cast<ofAppGLFWWindow*>(ofWindow.get());
    if (glfwWindow) {
      glfwMakeContextCurrent(glfwWindow->getGLFWWindow());
    }
  }
}

void EngineLoop::executeOnEngineThread(const std::function<void()>& task) {
  std::lock_guard<std::mutex> lock(taskQueueMutex);
  taskQueue.push(task);
}

void EngineLoop::runQueuedTasks() {
  std::queue<std::function<void()>> tasksToRun;
  {
    std::lock_guard<std::mutex> lock(taskQueueMutex);
    std::swap(tasksToRun, taskQueue);
  }
  while (!tasksToRun.empty()) {
    auto task = tasksToRun.front();
    tasksToRun.pop();
    task();
  }
}
