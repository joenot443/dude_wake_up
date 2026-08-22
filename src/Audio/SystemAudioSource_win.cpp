// SystemAudioSource_win.cpp
// Non-macOS placeholder for SystemAudioSource. Real WASAPI implementation
// lands in Phase 3 (see WINDOWS_PORT_PLAN.md). Exists so the empty-shell
// Windows build links.

#ifndef TARGET_OSX

#include "SystemAudioSource.hpp"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("SystemAudioSource_win") << "stub: " << method
        << " — WASAPI implementation deferred to Phase 3";
      warned = true;
    }
  }
}

SystemAudioSource::SystemAudioSource()  {}
SystemAudioSource::~SystemAudioSource() {}

void SystemAudioSource::setup()   { warnOnce("setup"); }
void SystemAudioSource::disable() { warnOnce("disable"); }

void SystemAudioSource::setTargetProcess(pid_t pid) { targetPID = pid; warnOnce("setTargetProcess"); }
void SystemAudioSource::setGlobalTap(bool global)   { isGlobalTap = global; }

json SystemAudioSource::serialize()      { return json(); }
void SystemAudioSource::load(json /*j*/) { warnOnce("load"); }

#endif // !TARGET_OSX
