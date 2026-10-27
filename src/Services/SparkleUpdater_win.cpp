// SparkleUpdater_win.cpp
// Non-macOS placeholder for SparkleUpdaterService. Real WinSparkle wiring (or
// stub-out for v1) is decided in Phase 5 (see WINDOWS_PORT_PLAN.md).

#ifndef TARGET_OSX

#include "SparkleUpdater.h"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("SparkleUpdater_win") << "stub: " << method
        << " — auto-updater choice (WinSparkle vs stub) deferred to Phase 5";
      warned = true;
    }
  }
}

SparkleUpdaterService& SparkleUpdaterService::getInstance() {
  static SparkleUpdaterService inst;
  return inst;
}

void SparkleUpdaterService::initialize()                  { warnOnce("initialize"); }
void SparkleUpdaterService::checkForUpdates()             { warnOnce("checkForUpdates"); }
void SparkleUpdaterService::checkForUpdatesInBackground() { warnOnce("checkForUpdatesInBackground"); }
bool SparkleUpdaterService::canCheckForUpdates()          { return false; }

#endif // !TARGET_OSX
