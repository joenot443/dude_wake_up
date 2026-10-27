//
//  LibavRecorder.hpp
//
//  Windows-only libav (ffmpeg 8.1 LGPL) implementation of the recorder.
//  AVFRecorderWrapper_win.cpp forwards its public methods to an instance of
//  this class stored in `recorderInstance`. Kept out of the AVFRecorderWrapper
//  header so libav types (AVFormatContext, AVCodecContext, SwrContext, ...)
//  don't leak into every TU that records — they include AVFRecorderWrapper.hpp
//  and never see this file.
//
//  Threading:
//    - setup / start / stop     : caller thread (usually main)
//    - addFrame                 : caller thread (usually main draw loop)
//    - addAudioSamples          : audio thread (WASAPI / RtAudio callback)
//
//    writeMutex_ serializes every avformat_* and av_interleaved_write_frame
//    call. Video and audio paths encode independently but only ONE of them
//    is inside av_interleaved_write_frame at any moment (per libav muxer
//    thread-safety contract).
//
//    encoding state (SwsContext, SwrContext, per-encoder AVCodecContext) is
//    only ever touched by ONE of {video path, audio path} — never both —
//    so needs no extra mutex.
//

#ifndef LibavRecorder_hpp
#define LibavRecorder_hpp

#if defined(_WIN32) && !defined(TARGET_OSX)

#include "ofPixels.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class LibavRecorder {
public:
  LibavRecorder();
  ~LibavRecorder();

  LibavRecorder(const LibavRecorder&) = delete;
  LibavRecorder& operator=(const LibavRecorder&) = delete;

  bool setup(const std::string& filePath, int width, int height, float fps,
             const std::string& codec);
  bool setupAudio(float sampleRate, int channels);

  void setMaxDuration(float seconds) { maxDurationSeconds_ = seconds; }
  void setOnRecordingComplete(
      std::function<void(bool success, const std::string& filePath)> cb) {
    onComplete_ = std::move(cb);
  }

  bool start();
  bool addFrame(const ofPixels& pixels);
  bool addAudioSamples(const float* samples, int numFrames);
  void stop(std::function<void(bool success)> completionCallback);

  bool  isRecording()    const { return recording_.load(); }
  bool  hasAudio()       const { return audioEnabled_; }
  float getElapsedTime() const;
  float getMaxDuration() const { return maxDurationSeconds_; }

private:
  // Format + muxer
  AVFormatContext* formatCtx_ = nullptr;
  std::string      filePath_;
  bool             ioOpened_ = false;
  bool             headerWritten_ = false;

  // Video encoder
  AVStream*        videoStream_ = nullptr;
  AVCodecContext*  videoCodecCtx_ = nullptr;
  const AVCodec*   videoCodec_ = nullptr;
  SwsContext*      swsCtx_ = nullptr;
  AVFrame*         videoFrame_ = nullptr;   // reused per frame (YUV420P dest)
  AVPacket*        videoPacket_ = nullptr;
  int              srcWidth_ = 0, srcHeight_ = 0;
  int              dstWidth_ = 0, dstHeight_ = 0;
  int              fpsInt_ = 30;
  AVPixelFormat    srcPixelFormat_ = AV_PIX_FMT_RGBA;
  std::string      openedVideoEncoderName_;

  // Audio encoder
  AVStream*        audioStream_ = nullptr;
  AVCodecContext*  audioCodecCtx_ = nullptr;
  const AVCodec*   audioCodec_ = nullptr;
  SwrContext*      swrCtx_ = nullptr;
  AVFrame*         audioFrame_ = nullptr;   // reused per encoded chunk (FLTP)
  AVPacket*        audioPacket_ = nullptr;
  bool             audioEnabled_ = false;
  int              audioInSampleRate_ = 48000;
  int              audioInChannels_ = 2;
  int              audioOutSampleRate_ = 48000;
  int              audioOutChannels_ = 2;
  int              audioFrameSize_ = 1024;   // set by avcodec_open2 (AAC=1024)
  int64_t          audioSamplesEncoded_ = 0; // cumulative OUTPUT samples encoded (PTS clock)
  std::vector<float> audioTail_;             // interleaved float32 leftover < audioFrameSize_

  // Duration + completion
  float                                             maxDurationSeconds_ = 0.f;
  std::function<void(bool, const std::string&)>     onComplete_;
  std::atomic<bool>                                 completed_{false};
  std::atomic<bool>                                 recording_{false};
  std::chrono::steady_clock::time_point             startTime_;

  // Concurrency
  mutable std::mutex writeMutex_;   // guards every avformat_*, av_interleaved_write_frame
  mutable std::mutex audioMutex_;   // guards audioTail_ + swrCtx_ (audio path re-entry)

  // Helpers
  bool  openVideoEncoderLadder_(int width, int height, int fps);
  bool  buildOrRebuildSwrContext_(int inSampleRate, int inChannels);
  bool  encodeAndWriteAudioChunk_(const float* interleavedFloat, int inFrames);
  bool  drainEncoder_(AVCodecContext* ctx, AVStream* stream);
  void  finalizeIfLive_(bool success);
  void  freeAll_();
  int64_t elapsedNs_() const;

  static bool verifyLgplOnce_();
};

#endif // _WIN32 && !TARGET_OSX
#endif // LibavRecorder_hpp
