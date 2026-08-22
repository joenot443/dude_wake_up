#pragma once

#include <gtest/gtest.h>

#ifdef _WIN32

#include "Utf8Path.hpp"

// String literals are written as explicit UTF-8 byte escapes so the test is
// robust regardless of the source-file encoding assumed by the compiler
// (MSVC's default is the local ANSI codepage; /utf-8 flag makes it UTF-8).

// "日本語" (Japanese)  — U+65E5 U+672C U+8A9E
static const char kJapanese[] = "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E";
// "Юрий" (Cyrillic)     — U+042E U+0440 U+0438 U+0439
static const char kCyrillic[] = "\xD0\xAE\xD1\x80\xD0\xB8\xD0\xB9";
// "José" (Latin accent) — U+004A U+006F U+0073 U+00E9
static const char kLatinAccent[] = "Jos\xC3\xA9";

TEST(Utf8PathTests, RoundTrip_Japanese) {
  const std::string input(kJapanese);
  EXPECT_EQ(input, nottawa::wide_to_utf8(nottawa::utf8_to_wide(input)));
}

TEST(Utf8PathTests, RoundTrip_Cyrillic) {
  const std::string input(kCyrillic);
  EXPECT_EQ(input, nottawa::wide_to_utf8(nottawa::utf8_to_wide(input)));
}

TEST(Utf8PathTests, RoundTrip_LatinAccent) {
  const std::string input(kLatinAccent);
  EXPECT_EQ(input, nottawa::wide_to_utf8(nottawa::utf8_to_wide(input)));
}

TEST(Utf8PathTests, EmptyString) {
  EXPECT_TRUE(nottawa::utf8_to_wide("").empty());
  EXPECT_TRUE(nottawa::wide_to_utf8(std::wstring()).empty());
}

TEST(Utf8PathTests, WideLengthMatchesCodePoints) {
  // "José" is 4 code points, encoded in UTF-16 as 4 code units (no surrogates).
  EXPECT_EQ(nottawa::utf8_to_wide(std::string(kLatinAccent)).size(), 4u);
  // Japanese CJK chars all fit in the BMP → 3 code points, 3 UTF-16 code units.
  EXPECT_EQ(nottawa::utf8_to_wide(std::string(kJapanese)).size(), 3u);
}

#else

TEST(Utf8PathTests, SkippedOnNonWindows) {
  GTEST_SKIP() << "Utf8Path is Windows-only; UTF-16 is not needed on this platform.";
}

#endif // _WIN32
