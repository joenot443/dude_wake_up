#pragma once

#include <gtest/gtest.h>

// M4.3a — recorder integration test pulled forward from Phase 6. Isolates
// the libav muxing/encoding path from WASAPI capture so a broken recorder
// doesn't masquerade as a broken audio thread (or vice versa) when the
// full pipeline lands. Windows-only because LibavRecorder is Windows-only.

#ifdef _WIN32

#include "AVFRecorderWrapper.hpp"
#include "ofPixels.h"
#include "Utf8Path.hpp"

#include <atomic>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <thread>
#include <vector>

namespace {

// Fill an RGBA frame with a moving color-band so the encoder actually has
// bitrate to burn (a solid-color frame compresses to almost nothing and
// masks bugs in the sws / encoder path).
void paintFrame(ofPixels& px, int frameIdx) {
  const int w = px.getWidth();
  const int h = px.getHeight();
  auto* p = px.getData();
  const int stride = static_cast<int>(px.getBytesStride());
  for (int y = 0; y < h; ++y) {
    uint8_t* row = p + y * stride;
    for (int x = 0; x < w; ++x) {
      row[x * 4 + 0] = static_cast<uint8_t>((x + frameIdx * 3) & 0xFF);
      row[x * 4 + 1] = static_cast<uint8_t>((y + frameIdx * 5) & 0xFF);
      row[x * 4 + 2] = static_cast<uint8_t>((x + y + frameIdx) & 0xFF);
      row[x * 4 + 3] = 255;
    }
  }
}

// Generate `numFrames` interleaved stereo samples of a 1 kHz sine at
// `sampleRate` Hz starting from `phaseFrames` samples in.
void generateSine(std::vector<float>& out, int numFrames, int channels,
                  float sampleRate, int64_t phaseFrames) {
  out.resize(static_cast<size_t>(numFrames) * channels);
  const double omega = 2.0 * 3.14159265358979 * 1000.0 / sampleRate;
  for (int i = 0; i < numFrames; ++i) {
    float s = static_cast<float>(std::sin(omega * static_cast<double>(phaseFrames + i)));
    for (int c = 0; c < channels; ++c) {
      out[static_cast<size_t>(i) * channels + c] = s * 0.25f;
    }
  }
}

// Check the file's first 8 bytes look like an mp4 `ftyp` box (little-endian
// size prefix followed by ASCII 'ftyp'). Cheap smoke — proves the muxer
// wrote at least a container header.
bool fileLooksLikeMp4(const std::string& path) {
  std::FILE* f = std::fopen(path.c_str(), "rb");
  if (!f) return false;
  uint8_t hdr[8]{};
  const size_t n = std::fread(hdr, 1, sizeof(hdr), f);
  std::fclose(f);
  if (n < 8) return false;
  return hdr[4] == 'f' && hdr[5] == 't' && hdr[6] == 'y' && hdr[7] == 'p';
}

std::string scratchPath(const char* leafname) {
  auto dir = std::filesystem::temp_directory_path() / "nottawa_recorder_tests";
  std::filesystem::create_directories(dir);
  return (dir / leafname).string();
}

} // namespace

TEST(RecorderTests, VideoOnly_ProducesPlayableMp4) {
  const int W = 320, H = 180, FPS = 30, N = 60;
  const std::string outPath = scratchPath("video_only.mp4");
  std::filesystem::remove(outPath);

  AVFRecorderWrapper rec;
  ASSERT_TRUE(rec.setup(outPath, W, H, static_cast<float>(FPS), "h264"))
      << "setup failed — is ffmpeg present and LGPL-configured?";
  ASSERT_TRUE(rec.start());

  ofPixels frame;
  frame.allocate(W, H, OF_PIXELS_RGBA);
  for (int i = 0; i < N; ++i) {
    paintFrame(frame, i);
    ASSERT_TRUE(rec.addFrame(frame)) << "addFrame " << i;
    // Advance wall clock so wall-clock PTS produces distinct timestamps.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
  }

  std::atomic<bool> done{false}, ok{false};
  rec.stop([&](bool success) { ok = success; done = true; });
  // stop() runs the finalize inline in our impl, so `done` is already true.
  EXPECT_TRUE(done);
  EXPECT_TRUE(ok);
  EXPECT_FALSE(rec.isRecording());
  EXPECT_TRUE(fileLooksLikeMp4(outPath)) << outPath;
  auto size = std::filesystem::file_size(outPath);
  EXPECT_GT(size, static_cast<uintmax_t>(4096))
      << "output file is suspiciously small (" << size << " bytes)";
}

TEST(RecorderTests, VideoAndAudio_ProducesPlayableMp4) {
  const int W = 320, H = 180, FPS = 30, N = 30;
  const float SR = 48000.f;
  const int CH = 2;
  const std::string outPath = scratchPath("video_audio.mp4");
  std::filesystem::remove(outPath);

  AVFRecorderWrapper rec;
  ASSERT_TRUE(rec.setup(outPath, W, H, static_cast<float>(FPS), "h264"));
  ASSERT_TRUE(rec.setupAudio(SR, CH));
  ASSERT_TRUE(rec.start());

  ofPixels frame;
  frame.allocate(W, H, OF_PIXELS_RGBA);
  std::vector<float> sine;
  int64_t phase = 0;
  // Roughly 1 second of a/v: 30 frames @ 30fps, sine chunks of 1024 samples
  // (matches AAC frame_size for stress on the audioTail path).
  for (int i = 0; i < N; ++i) {
    paintFrame(frame, i);
    ASSERT_TRUE(rec.addFrame(frame));
    generateSine(sine, 1024, CH, SR, phase);
    phase += 1024;
    ASSERT_TRUE(rec.addAudioSamples(sine.data(), 1024));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
  }

  bool ok = false;
  rec.stop([&](bool success) { ok = success; });
  EXPECT_TRUE(ok);
  EXPECT_TRUE(fileLooksLikeMp4(outPath)) << outPath;
  auto size = std::filesystem::file_size(outPath);
  EXPECT_GT(size, static_cast<uintmax_t>(8192));
}

TEST(RecorderTests, DurationLimit_FinalizesOnce) {
  const int W = 160, H = 90, FPS = 60;
  const std::string outPath = scratchPath("duration_limit.mp4");
  std::filesystem::remove(outPath);

  AVFRecorderWrapper rec;
  ASSERT_TRUE(rec.setup(outPath, W, H, static_cast<float>(FPS), "h264"));
  rec.setMaxDuration(0.15f); // 150ms cap

  std::atomic<int> completionCount{0};
  rec.setOnRecordingComplete(
    [&](bool /*ok*/, const std::string& /*p*/) { completionCount++; });

  ASSERT_TRUE(rec.start());
  ofPixels frame;
  frame.allocate(W, H, OF_PIXELS_RGBA);
  // Push frames well past the cap. Once we're past 150ms of wall-clock the
  // recorder should self-finalize on the NEXT addFrame call.
  for (int i = 0; i < 30; ++i) {
    paintFrame(frame, i);
    rec.addFrame(frame);   // return value ignored — false expected after cap
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  // Explicit stop AFTER auto-finalize should NOT fire the callback again.
  rec.stop(nullptr);
  EXPECT_EQ(completionCount.load(), 1) << "onRecordingComplete must fire exactly once";
  EXPECT_TRUE(fileLooksLikeMp4(outPath));
}

TEST(RecorderTests, UnicodeOutputPath) {
  // "Nottawa 日本語.mp4" — validates the manifest-based UTF-8 path policy
  // reaches all the way through libav's file protocol.
  const std::string leaf = "Nottawa \xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E.mp4";
  const std::string outPath = scratchPath(leaf.c_str());
  std::filesystem::remove(std::filesystem::u8path(outPath));

  AVFRecorderWrapper rec;
  ASSERT_TRUE(rec.setup(outPath, 160, 90, 30.f, "h264"));
  ASSERT_TRUE(rec.start());
  ofPixels frame;
  frame.allocate(160, 90, OF_PIXELS_RGBA);
  for (int i = 0; i < 10; ++i) {
    paintFrame(frame, i);
    rec.addFrame(frame);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
  rec.stop(nullptr);
  EXPECT_TRUE(fileLooksLikeMp4(outPath))
      << "unicode path " << outPath
      << " — did the manifest activeCodePage=UTF-8 make it into the build?";
}

#else

TEST(RecorderTests, SkippedOnNonWindows) {
  GTEST_SKIP() << "LibavRecorder is Windows-only; macOS uses AVFoundation directly.";
}

#endif // _WIN32
