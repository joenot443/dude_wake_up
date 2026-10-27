#pragma once

#include <QString>

// A plain-text summary for bug reports (Settings > Advanced > Copy
// Diagnostics): the version, system, graphics, displays, engine state,
// settings changed from their defaults, and whether the previous session
// ended with a crash or hang report (logs/engine.previous.log).
namespace Diagnostics {
QString report();
}
