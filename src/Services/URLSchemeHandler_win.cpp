// URLSchemeHandler_win.cpp
// Non-macOS placeholder for URLSchemeHandler. Real Windows deep-link support
// (HKCR registry entry + argv parsing on cold launch + single-instance
// named-pipe forwarding on warm launch) lands in Phase 5.

#ifndef TARGET_OSX

#include "URLSchemeHandler.h"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("URLSchemeHandler_win") << "stub: " << method
        << " — registry + argv + named-pipe wiring deferred to Phase 5";
      warned = true;
    }
  }
}

URLSchemeHandler& URLSchemeHandler::getInstance() {
  static URLSchemeHandler inst;
  return inst;
}

void URLSchemeHandler::initialize()                                          { warnOnce("initialize"); initialized = true; }
void URLSchemeHandler::processPendingURL()                                   { /* silent — called per-frame in some flows */ }
void URLSchemeHandler::handleURL(const std::string& url)                     { warnOnce("handleURL"); pendingURL = url; }
void URLSchemeHandler::setStrandLoadedCallback(StrandLoadedCallback cb)      { strandLoadedCallback = std::move(cb); }

#endif // !TARGET_OSX
