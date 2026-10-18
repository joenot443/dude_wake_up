#pragma once

// On an unhandled exception, writes the exception and a symbolised stack
// trace to stderr (which main.cpp sends to <app data>/logs/engine.log),
// then lets Windows' default handling continue.
namespace CrashHandler {
void install();
}
