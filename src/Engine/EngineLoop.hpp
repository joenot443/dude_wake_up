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

class EngineLoop {
public:
  static EngineLoop* getInstance();

  // Initialize all services in dependency order.
  // Must be called from a thread with a valid OpenGL context.
  void setup();

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
