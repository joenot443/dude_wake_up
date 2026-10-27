//
//  AudioRawSink.cpp
//

#include "AudioRawSink.hpp"

#ifndef NDEBUG

#include "ofLog.h"
#include <cstring>

#ifdef _WIN32
#include "Utf8Path.hpp"
#include <cstdio>
#endif

namespace {
std::FILE* openBinaryWrite(const std::string& utf8Path) {
#ifdef _WIN32
  std::wstring wide = nottawa::utf8_to_wide(utf8Path);
  if (wide.empty() && !utf8Path.empty()) {
    ofLogError("AudioRawSink") << "utf8_to_wide failed for path: " << utf8Path;
    return nullptr;
  }
  std::FILE* f = nullptr;
  errno_t err = _wfopen_s(&f, wide.c_str(), L"wb");
  if (err != 0) {
    ofLogError("AudioRawSink") << "_wfopen_s failed err=" << err << " path=" << utf8Path;
    return nullptr;
  }
  return f;
#else
  std::FILE* f = std::fopen(utf8Path.c_str(), "wb");
  if (!f) {
    ofLogError("AudioRawSink") << "fopen failed for path: " << utf8Path;
  }
  return f;
#endif
}
} // namespace

bool AudioRawSink::start(std::shared_ptr<AudioSource> source,
                         const std::string& outPath) {
  if (!source) {
    ofLogError("AudioRawSink") << "start called with null source";
    return false;
  }
  if (capturing_.load()) {
    ofLogWarning("AudioRawSink") << "already capturing to " << path_;
    return false;
  }

  std::FILE* f = openBinaryWrite(outPath);
  if (!f) return false;

  {
    std::lock_guard<std::mutex> lock(fileMutex_);
    file_ = f;
    path_ = outPath;
    bytesWritten_ = 0;
    channels_     = 0;
    sampleRate_   = 0.0f;
  }
  source_    = source;
  capturing_ = true;

  // Install the callback. Runs on the audio thread. Keep it lean — no
  // logging, no allocations, no lock contention beyond the small fileMutex_.
  auto self = this;   // stable across the source lifetime we own
  source->setRecordingCallback(
    [self](const float* samples, int numFrames, int numChannels, float sr) {
      if (!self->capturing_.load()) return;
      // Capture metadata on first invocation.
      if (self->channels_.load() == 0) {
        self->channels_   = numChannels;
        self->sampleRate_ = sr;
      }
      const size_t bytes = static_cast<size_t>(numFrames) * numChannels * sizeof(float);
      std::lock_guard<std::mutex> lock(self->fileMutex_);
      if (self->file_ && samples) {
        std::fwrite(samples, 1, bytes, self->file_);
        self->bytesWritten_.fetch_add(bytes);
      }
    });

  ofLogNotice("AudioRawSink") << "capturing to " << outPath
    << " — stop() to finalize, then:";
  ofLogNotice("AudioRawSink") << "  ffplay -f f32le -ar <rate> -ac <channels> \""
                              << outPath << "\"";
  return true;
}

void AudioRawSink::stop() {
  if (!capturing_.exchange(false)) return;

  if (source_) {
    source_->clearRecordingCallback();
    source_.reset();
  }

  std::lock_guard<std::mutex> lock(fileMutex_);
  if (file_) {
    std::fflush(file_);
    std::fclose(file_);
    file_ = nullptr;
  }
  ofLogNotice("AudioRawSink") << "finalized " << path_
    << " (" << bytesWritten_.load() << " bytes, "
    << channels_.load() << "ch @ " << sampleRate_.load() << " Hz)";
  ofLogNotice("AudioRawSink") << "inspect:  ffplay -f f32le -ar "
    << sampleRate_.load() << " -ac " << channels_.load()
    << " \"" << path_ << "\"";
}

#endif // !NDEBUG
