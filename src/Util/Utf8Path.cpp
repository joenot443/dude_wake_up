#include "Utf8Path.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>

namespace nottawa {

std::wstring utf8_to_wide(const std::string& utf8) {
  if (utf8.empty()) return std::wstring();
  const int len = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                        utf8.data(), static_cast<int>(utf8.size()),
                                        nullptr, 0);
  if (len <= 0) throw std::runtime_error("utf8_to_wide: invalid UTF-8 input");
  std::wstring out(static_cast<size_t>(len), L'\0');
  ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                        utf8.data(), static_cast<int>(utf8.size()),
                        &out[0], len);
  return out;
}

std::string wide_to_utf8(const std::wstring& wide) {
  if (wide.empty()) return std::string();
  const int len = ::WideCharToMultiByte(CP_UTF8, 0,
                                        wide.data(), static_cast<int>(wide.size()),
                                        nullptr, 0, nullptr, nullptr);
  if (len <= 0) throw std::runtime_error("wide_to_utf8: conversion failed");
  std::string out(static_cast<size_t>(len), '\0');
  ::WideCharToMultiByte(CP_UTF8, 0,
                        wide.data(), static_cast<int>(wide.size()),
                        &out[0], len, nullptr, nullptr);
  return out;
}

} // namespace nottawa

#endif // _WIN32
