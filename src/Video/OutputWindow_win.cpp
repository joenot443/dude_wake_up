// OutputWindow_win.cpp
// Non-macOS placeholder for OutputWindow's NSMenu methods. Real Windows menu
// (ImGui-based or Win32 HMENU) lands in Phase 5. The window itself (ofFbo,
// draw loop, recording plumbing) is cross-platform via OF; these are just
// the platform-specific bits.

#ifndef TARGET_OSX

#include "OutputWindow.hpp"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("OutputWindow_win") << "stub: " << method
        << " — native menu replacement deferred to Phase 5";
      warned = true;
    }
  }
}

void OutputWindow::setup()             {}
void OutputWindow::update()            {}
void OutputWindow::draw()              {}
void OutputWindow::teardown()          {}
void OutputWindow::keyReleased(int)    {}
void OutputWindow::keyPressed(int)     {}
void OutputWindow::updateResolution()  { needsResolutionUpdate = false; }
void OutputWindow::startRecording()    { warnOnce("startRecording"); }
void OutputWindow::stopRecording()     { warnOnce("stopRecording"); }
void OutputWindow::setupNativeMenu()   { warnOnce("setupNativeMenu"); }
void OutputWindow::updateMenuState()   {}

#endif // !TARGET_OSX
