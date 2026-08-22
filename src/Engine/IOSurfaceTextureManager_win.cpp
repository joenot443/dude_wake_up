// IOSurfaceTextureManager_win.cpp
// Non-macOS placeholder. Real DXGI-shared-handle implementation lands in
// Phase 2 (see WINDOWS_PORT_PLAN.md). Exists so the empty-shell Windows
// build links; texture sharing is a no-op until Phase 2.

#ifndef TARGET_OSX

#include "IOSurfaceTextureManager.hpp"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("IOSurfaceTextureManager_win") << "stub: " << method
        << " — DXGI shared-handle implementation deferred to Phase 2";
      warned = true;
    }
  }
}

IOSurfaceTextureManager* IOSurfaceTextureManager::instance = nullptr;

IOSurfaceTextureManager::~IOSurfaceTextureManager() {}

IOSurfaceTextureManager* IOSurfaceTextureManager::getInstance() {
  if (!instance) instance = new IOSurfaceTextureManager();
  return instance;
}

void IOSurfaceTextureManager::startSharing(const std::string& /*id*/)                             { warnOnce("startSharing"); }
void IOSurfaceTextureManager::stopSharing(const std::string& /*id*/)                              { /* silent no-op */ }
void IOSurfaceTextureManager::blitTrackedTextures()                                                { /* silent no-op — called per-frame; don't spam */ }
void IOSurfaceTextureManager::blitExternalFrame(const std::string&, unsigned int, int, int)       { /* silent no-op — called per-frame */ }

IOSurfaceRef IOSurfaceTextureManager::getIOSurface(const std::string& /*id*/) { return nullptr; }
int          IOSurfaceTextureManager::getWidth   (const std::string& /*id*/) { return 0; }
int          IOSurfaceTextureManager::getHeight  (const std::string& /*id*/) { return 0; }
bool         IOSurfaceTextureManager::isSharing  (const std::string& /*id*/) { return false; }

void IOSurfaceTextureManager::setSecondaryLookup(ConnectableLookup lookup) { secondaryLookup = std::move(lookup); }

#endif // !TARGET_OSX
