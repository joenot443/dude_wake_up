//
//  EngineLoop.cpp
//  dude_wake_up
//
//  Standalone engine update loop extracted from MainApp::update().
//

#include "EnginePreferences.hpp"
#include "EngineLoop.hpp"
#include "AudioSourceService.hpp"
#include "BookmarkService.hpp"
#include "ConfigService.hpp"
#include "FontService.hpp"
#include "ImageService.hpp"
#include "LayoutStateService.hpp"
#include "MidiService.hpp"
#include "MidiSmoothingService.hpp"
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
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef _WIN32
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "GLFW/glfw3native.h"
#include <filesystem>
#endif
#include <sys/stat.h>
#include <chrono>
#include <cstdlib>
#include <thread>

// Hidden GLFW window for OpenGL context (NottawaApp only).
static GLFWwindow* hiddenWindow = nullptr;

EngineLoop* EngineLoop::instance = nullptr;

EngineLoop* EngineLoop::getInstance() {
  if (!instance) {
    instance = new EngineLoop();
  }
  return instance;
}

static std::string dataPathOverride;
static void* hostShareContext = nullptr;

void EngineLoop::setDataPath(const std::string& path) { dataPathOverride = path; }
void EngineLoop::setShareContext(void* nativeContext) { hostShareContext = nativeContext; }

static void setupDataPath() {
  if (!dataPathOverride.empty()) {
    std::string path = dataPathOverride;
    if (path.back() != '/' && path.back() != '\\') path += '/';
    ofSetDataPathRoot(path);
    return;
  }
#ifdef __APPLE__
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
#endif
}

/// Create a directory if it doesn't exist using POSIX stat/mkdir.
/// OF's ofDirectory::createDirectory fails silently on paths with spaces
/// (e.g. "Application Support"), so we use POSIX directly for NottawaApp.
static void ensureDirectory(const std::string& path) {
#if defined(_WIN32)
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::u8path(path), ec);
#elif defined(NOTTAWA_ENGINE_ONLY)
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

#ifdef _WIN32
// Lets a raw GLFWwindow be passed as ofGLFWWindowSettings::shareContextWith,
// which OF only reads through getWindowContext().
class BareShareHandle : public ofAppBaseWindow {
public:
  explicit BareShareHandle(GLFWwindow* w) : raw(w) {}
  void* getWindowContext() override { return raw; }
  void setup(const ofWindowSettings&) override {}
  void update() override {}
  void draw() override {}
  ofCoreEvents& events() override { return coreEvents; }
  std::shared_ptr<ofBaseRenderer>& renderer() override { return noRenderer; }
private:
  GLFWwindow* raw;
  ofCoreEvents coreEvents;
  std::shared_ptr<ofBaseRenderer> noRenderer;
};

static GLFWwindow* shareRoot = nullptr;
static std::shared_ptr<BareShareHandle> shareRootHandle;

// Joins the host UI's GL context (an HGLRC) to the engine's share group so
// textures the engine renders are directly usable by the UI. wglShareLists
// refuses contexts that already contain objects, and OF populates its
// context during ofCreateWindow, so: create an empty GLFW context, share it
// with the host while both are empty, then create OF's window sharing with
// that root.
static bool createShareRoot() {
  if (!glfwInit()) return false;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  shareRoot = glfwCreateWindow(16, 16, "nottawa-share-root", nullptr, nullptr);
  glfwDefaultWindowHints();
  if (!shareRoot) return false;
  if (!wglShareLists(static_cast<HGLRC>(hostShareContext), glfwGetWGLContext(shareRoot))) {
    ofLogError("EngineLoop") << "wglShareLists failed: " << GetLastError();
    glfwDestroyWindow(shareRoot);
    shareRoot = nullptr;
    return false;
  }
  shareRootHandle = std::make_shared<BareShareHandle>(shareRoot);
  return true;
}
#endif

static bool setupOpenGLContext() {
  // Create a hidden OF window for full GL initialization.
  // OF needs its own window creation path to set up the programmable
  // renderer, extension detection, FBO support, etc.
  ofGLFWWindowSettings settings;
  settings.setGLVersion(3, 2);
  settings.setSize(64, 64);
#ifdef _WIN32
  settings.visible = false;
  if (hostShareContext) {
    if (!createShareRoot()) return false;
    settings.shareContextWith = shareRootHandle;
  }
#endif

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

  // App preferences (apart from the session; seeded from it on first run),
  // then the last session.
  EnginePreferences::load();
  ConfigService::getService()->loadDefaultConfigFile();
  EnginePreferences::setup();

  StrandService::getService()->setup();

#ifndef NOTTAWA_ENGINE_ONLY
  ImageService::getService()->setup();
#endif

  ParameterService::getService()->setup();
  TimeService::getService()->setup();

  StringManager::loadStrings();

  setupComplete = true;

  // Auto-run leak test if NTW_LEAK_TEST=N is set in the environment.
  // The test runs on a detached thread; bridge functions dispatch back to
  // the engine thread for actual mutations.
  if (const char* env = std::getenv("NTW_LEAK_TEST")) {
    int cycles = std::atoi(env);
    if (cycles > 0) {
      ofLogNotice("EngineLoop") << "NTW_LEAK_TEST=" << cycles << " — running leak test in 3s";
      std::thread([cycles]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        char* result = ntw_run_leak_test(cycles);
        if (result) {
          ofLogNotice("EngineLoop") << "Leak test complete";
          free(result);
        }
      }).detach();
    }
  }
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

  // A frame over 250 ms is logged with where the time went (the UI waits
  // on the engine thread, so these are the app's stalls). GPU work can land
  // in a later stage than the one that queued it.
  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();

  // 1. Run queued tasks
  runQueuedTasks();
  const auto tTasks = clock::now();

  // 2. Update oscillators and modulation
  OscillationService::getService()->tickOscillators();
  ModulationService::getService()->tickMappings();

  // 3. Update parameters
  ParameterService::getService()->tickParameters();

  // 3b. Advance MIDI smoothings (eases stepped CC values toward their target).
  MidiSmoothingService::getService()->tick();

  const auto tUpdate = clock::now();

  // 4. Update video sources (render to FBOs)
  VideoSourceService::getService()->updateVideoSources();
  const auto tSources = clock::now();

  // 5. Process shader chain (render effects)
  ShaderChainerService::getService()->processFrame();
  const auto tEffects = clock::now();

  // 5b. Tick preview instances (browser live previews)
  ntw_tick_previews();
  const auto tPreviews = clock::now();

  // 5c. Blit tracked textures to IOSurfaces (no-op when nothing tracked)
  IOSurfaceTextureManager::getInstance()->blitTrackedTextures();

  // 6. Advance global time
  TimeService::getService()->tick();
  const auto tBlit = clock::now();

  // 7. Auto-save config if needed
#ifdef NOTTAWA_ENGINE_ONLY
  ConfigService::getService()->flushRequestedSave();
  // ofGetFrameNum() is always 0 in NottawaApp (no OF main loop), so
  // ConfigService::checkAndSaveDefaultConfigFile() never triggers.
  // Use time-based auto-save instead.
  if (now - lastAutoSaveTime >= autoSaveInterval && now >= 10.0) {
    lastAutoSaveTime = now;
    auto cs = ConfigService::getService();
    auto cfg = cs->currentConfig();
    if (cfg != cs->lastConfig) {
      cs->saveDefaultConfigFile();
    }
  }
#else
  ConfigService::getService()->checkAndSaveDefaultConfigFile();
#endif

  const auto tSave = clock::now();

  // 8. Update audio analysis
  AudioSourceService::getService()->update();

  // 9. Update MIDI
  MidiService::getService()->update();

  const auto tEnd = clock::now();
  auto ms = [](clock::time_point a, clock::time_point b) {
    return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count());
  };
  if (ms(t0, tEnd) > 250) {
    printf("[engine] slow frame %d ms: tasks %d, update %d, sources %d, effects %d, previews %d, blit %d, save %d, "
           "audio+midi %d\n",
           ms(t0, tEnd), ms(t0, tTasks), ms(tTasks, tUpdate), ms(tUpdate, tSources), ms(tSources, tEffects),
           ms(tEffects, tPreviews), ms(tPreviews, tBlit), ms(tBlit, tSave), ms(tSave, tEnd));
    fflush(stdout);
  }
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
