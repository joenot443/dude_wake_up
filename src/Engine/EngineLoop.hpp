//
//  EngineLoop.hpp
//  dude_wake_up
//
//  Standalone engine update loop extracted from MainApp::update().
//  Drives the C++ rendering pipeline without any UI dependencies.
//

#ifndef EngineLoop_hpp
#define EngineLoop_hpp

#include <functional>
#include <mutex>
#include <queue>
#include <string>

class EngineLoop {
public:
  static EngineLoop* getInstance();

  // Initialize all services in dependency order.
  // Must be called from a thread with a valid OpenGL context.
  void setup();

  // Optional, before setup(). Overrides the OF data root (otherwise the
  // macOS bundle's Resources/data/).
  void setDataPath(const std::string& path);

  // Optional, before setup(). Native GL context of the host UI (Windows:
  // HGLRC) to join to the engine's share group, so textures shared through
  // IOSurfaceTextureManager are usable directly in the UI's context.
  void setShareContext(void* nativeContext);

  // Run one frame of the engine pipeline:
  //   oscillators -> modulation -> parameters -> video sources ->
  //   shader chain -> time -> config autosave -> audio -> midi
  void tick();

  // Queue a task to run on the engine thread at the start of the next tick.
  void executeOnEngineThread(const std::function<void()>& task);

  // Release the OpenGL context from the current thread (call before
  // handing off to the engine thread).
  void releaseContext();

  // Make the OpenGL context current on the calling thread (call at the
  // start of the engine thread).
  void makeContextCurrent();

  // Whether setup() has been called
  bool isSetup() const { return setupComplete; }

  // Engine FPS (ticks per second)
  float currentFPS() const { return fps; }

private:
  EngineLoop() = default;

  void runQueuedTasks();

  bool setupComplete = false;
  std::queue<std::function<void()>> taskQueue;
  std::mutex taskQueueMutex;

  // FPS tracking
  float fps = 0.0f;
  int frameCount = 0;
  double lastFPSTime = 0.0;

  // Auto-save timing (time-based, works without OF main loop)
  double lastAutoSaveTime = 0.0;
  static constexpr double autoSaveInterval = 50.0; // seconds

  static EngineLoop* instance;
};

#endif /* EngineLoop_hpp */
