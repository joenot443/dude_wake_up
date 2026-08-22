// Canary translation unit — exists solely to force the NOMINMAX check in
// PortabilityCheck.hpp to run against a TU that has already pulled in
// <Windows.h>. On non-Windows builds this is an empty TU.
//
// If a build ever strips NOMINMAX from the Windows project settings, this
// file will fail to compile with a message pointing straight at the fix.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "PortabilityCheck.hpp"
#endif
