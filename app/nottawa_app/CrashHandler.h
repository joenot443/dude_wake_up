#pragma once

// On an unhandled exception, writes the exception and a symbolised stack
// trace to stderr (which main.cpp sends to <app data>/logs/engine.log),
// then lets Windows' default handling continue.
namespace CrashHandler {
void install();

// Hang reports: once the UI thread's heartbeat (call heartbeat() from a UI
// timer) has been silent for `stallMs`, every thread's stack is written to
// stderr, once per stall. Call startHangWatchdog() on the UI thread.
void startHangWatchdog(int stallMs);
void heartbeat();
}
