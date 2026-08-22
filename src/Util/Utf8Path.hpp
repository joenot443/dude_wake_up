#pragma once

// UTF-8 <-> UTF-16 conversion helpers for calling Win32 wide-character APIs.
//
// Rule at the Windows boundary: all in-app strings are UTF-8 (std::string).
// Cross into Win32 only through utf8_to_wide(); cross back only through
// wide_to_utf8(). Never call the *A() variants of Win32 APIs from Windows-
// specific code — they use the local ANSI codepage and silently corrupt paths
// with non-ASCII characters (Cyrillic usernames, Japanese library names, etc.).
//
// On non-Windows platforms this header intentionally exposes nothing: UTF-16
// is a Windows-only OS interchange format and no other platform needs it.

#ifdef _WIN32

#include <string>

namespace nottawa {

// Convert a UTF-8 string to a UTF-16 wide string. Throws on invalid UTF-8.
std::wstring utf8_to_wide(const std::string& utf8);

// Convert a UTF-16 wide string to UTF-8. Throws on conversion failure.
std::string  wide_to_utf8(const std::wstring& wide);

} // namespace nottawa

#endif // _WIN32
