//
//  AudioRawSink.hpp
//  Dev-only helper that writes raw interleaved float32 PCM samples out to
//  a .raw file via the AudioSource::setRecordingCallback wire. Lets us
//  prove the audio callback path is glitch-free BEFORE Phase 4 introduces
//  the libav recorder.
//
//  Usage:
//    auto sink = std::make_shared<AudioRawSink>();
//    sink->start(source, "capture.raw");
//    // ... wait 5s while WASAPI captures ...
//    sink->stop();
//
//  Inspect:
//    ffplay -f f32le -ar <sampleRate> -ac <channels> capture.raw
//    audacity → File > Import > Raw Data → 32-bit float, little-endian
//
//  Not compiled into release builds — see the #ifndef NDEBUG gate below.
//

#ifndef AudioRawSink_hpp
#define AudioRawSink_hpp

#ifndef NDEBUG

#include "AudioSource.hpp"
#include <atomic>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>

class AudioRawSink {
public:
  AudioRawSink() = default;
  ~AudioRawSink() { stop(); }

  AudioRawSink(const AudioRawSink&) = delete;
  AudioRawSink& operator=(const AudioRawSink&) = delete;

  // Starts capturing from `source` into `outPath`. `outPath` is UTF-8; on
  // Windows we open via _wfopen so Unicode filenames work. Returns true if
  // the file opened successfully AND the callback was installed. If false,
  // check the log — the source may already have a recording callback bound.
  bool start(std::shared_ptr<AudioSource> source, const std::string& outPath);

  // Detach the callback and close the file. Idempotent.
  void stop();

  bool isCapturing() const { return capturing_.load(); }

  // Written bytes across the whole capture. Grows on the audio thread.
  uint64_t bytesWritten() const { return bytesWritten_.load(); }

  // Captured metadata — populated on the first callback.
  int   channels()   const { return channels_.load(); }
  float sampleRate() const { return sampleRate_.load(); }

private:
  std::shared_ptr<AudioSource> source_;
  std::mutex        fileMutex_;
  std::FILE*        file_ = nullptr;
  std::string       path_;
  std::atomic<bool>     capturing_{false};
  std::atomic<uint64_t> bytesWritten_{0};
  std::atomic<int>      channels_{0};
  std::atomic<float>    sampleRate_{0.0f};
};

#endif // !NDEBUG
#endif // AudioRawSink_hpp
