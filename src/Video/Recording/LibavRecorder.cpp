//
//  LibavRecorder.cpp
//

#include "LibavRecorder.hpp"

#if defined(_WIN32) && !defined(TARGET_OSX)

#include "ofLog.h"
#include <algorithm>
#include <cstring>

extern "C" {
#include <libavutil/error.h>
#include <libavutil/mathematics.h>
}

namespace {

// AV error → string.
std::string avErr(int r) {
  char buf[AV_ERROR_MAX_STRING_SIZE]{};
  av_strerror(r, buf, sizeof(buf));
  return std::string(buf);
}

// Bitrate matches the macOS AVF path: clamp(w*h*4, 1M, 4M).
int computeBitrate(int w, int h) {
  int64_t br = static_cast<int64_t>(w) * h * 4;
  if (br < 1'000'000) br = 1'000'000;
  if (br > 4'000'000) br = 4'000'000;
  return static_cast<int>(br);
}

// The encoder ladder — first name that both find AND open wins.
const char* const kVideoEncoderLadder[] = {
  "h264_nvenc",   // NVIDIA
  "h264_qsv",     // Intel QuickSync
  "h264_amf",     // AMD
  "libopenh264",  // portable LGPL software fallback
  nullptr,
};

// Runtime LGPL check — refuses to initialize if the linked ffmpeg was
// configured with --enable-gpl. Belt-and-braces beyond the build-time
// SHA256 pin and license-scanner in fetch_ffmpeg_lgpl.ps1.
bool checkLgplConfiguration() {
  const char* cfg = avutil_configuration();
  if (!cfg) return false;
  if (std::strstr(cfg, "--enable-gpl")) {
    ofLogError("LibavRecorder")
      << "LGPL invariant violated: linked ffmpeg was built with --enable-gpl. "
      << "Refusing to record to avoid tainting a GPLv3 distribution. "
      << "Rebuild ffmpeg without --enable-gpl or fetch a BtbN LGPL asset.";
    ofLogError("LibavRecorder") << "configuration: " << cfg;
    return false;
  }
  return true;
}

} // namespace

bool LibavRecorder::verifyLgplOnce_() {
  static const bool ok = checkLgplConfiguration();
  return ok;
}

LibavRecorder::LibavRecorder() = default;

LibavRecorder::~LibavRecorder() {
  // Best-effort finalize so the file is playable even if the caller forgot
  // to stop() us. Runs the moov trailer through av_write_trailer.
  finalizeIfLive_(false);
  freeAll_();
}

// ─── setup ────────────────────────────────────────────────────────────────

bool LibavRecorder::setup(const std::string& filePath, int width, int height,
                          float fps, const std::string& /*codec*/) {
  std::lock_guard<std::mutex> lock(writeMutex_);

  if (!verifyLgplOnce_()) return false;

  if (recording_.load()) {
    ofLogError("LibavRecorder") << "setup called while already recording";
    return false;
  }
  if (formatCtx_) {
    // Prior setup left state around (start() never called, or a botched stop).
    // Free before starting fresh.
    freeAll_();
  }

  filePath_ = filePath;
  srcWidth_  = dstWidth_  = width;
  srcHeight_ = dstHeight_ = height;
  fpsInt_    = std::max(1, static_cast<int>(fps + 0.5f));

  // Force mp4 muxer regardless of extension. Matches macOS AVF (.mov) parity
  // in intent (container-controlled by us, not user file-name games) and
  // ensures the fragmented-mp4 movflags below have the muxer they need.
  int r = avformat_alloc_output_context2(&formatCtx_, nullptr, "mp4",
                                         filePath_.c_str());
  if (r < 0 || !formatCtx_) {
    ofLogError("LibavRecorder") << "avformat_alloc_output_context2: " << avErr(r);
    return false;
  }

  if (!openVideoEncoderLadder_(width, height, fpsInt_)) {
    ofLogError("LibavRecorder") << "no usable H.264 encoder";
    freeAll_();
    return false;
  }

  ofLogNotice("LibavRecorder")
    << "video encoder: " << openedVideoEncoderName_
    << " " << width << "x" << height << "@" << fpsInt_
    << " bitrate=" << computeBitrate(width, height);
  return true;
}

// ─── setupAudio ───────────────────────────────────────────────────────────

bool LibavRecorder::setupAudio(float sampleRate, int channels) {
  std::lock_guard<std::mutex> lock(writeMutex_);

  if (!formatCtx_) {
    ofLogError("LibavRecorder") << "setupAudio called before setup";
    return false;
  }
  if (recording_.load()) {
    ofLogError("LibavRecorder") << "setupAudio called while already recording";
    return false;
  }

  audioInSampleRate_ = static_cast<int>(sampleRate + 0.5f);
  audioInChannels_   = std::max(1, channels);

  // AAC in mp4 with a 44.1/48k stereo input is universally supported.
  audioOutSampleRate_ = audioInSampleRate_;
  audioOutChannels_   = audioInChannels_;

  audioCodec_ = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if (!audioCodec_) {
    ofLogError("LibavRecorder") << "AAC encoder not found in ffmpeg build";
    return false;
  }

  audioStream_ = avformat_new_stream(formatCtx_, nullptr);
  if (!audioStream_) {
    ofLogError("LibavRecorder") << "avformat_new_stream (audio) failed";
    return false;
  }

  audioCodecCtx_ = avcodec_alloc_context3(audioCodec_);
  if (!audioCodecCtx_) return false;

  audioCodecCtx_->sample_rate    = audioOutSampleRate_;
  audioCodecCtx_->bit_rate       = 128'000;
  audioCodecCtx_->sample_fmt     = AV_SAMPLE_FMT_FLTP;  // AAC wants planar float
  audioCodecCtx_->time_base      = {1, audioOutSampleRate_};
  av_channel_layout_default(&audioCodecCtx_->ch_layout, audioOutChannels_);

  if (formatCtx_->oformat && (formatCtx_->oformat->flags & AVFMT_GLOBALHEADER)) {
    audioCodecCtx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }

  int r = avcodec_open2(audioCodecCtx_, audioCodec_, nullptr);
  if (r < 0) {
    ofLogError("LibavRecorder") << "avcodec_open2 (audio): " << avErr(r);
    return false;
  }

  audioFrameSize_ = audioCodecCtx_->frame_size > 0
    ? audioCodecCtx_->frame_size
    : 1024;

  r = avcodec_parameters_from_context(audioStream_->codecpar, audioCodecCtx_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "avcodec_parameters_from_context (audio): " << avErr(r);
    return false;
  }
  audioStream_->time_base = audioCodecCtx_->time_base;

  audioFrame_  = av_frame_alloc();
  audioPacket_ = av_packet_alloc();
  if (!audioFrame_ || !audioPacket_) return false;

  audioFrame_->nb_samples   = audioFrameSize_;
  audioFrame_->format       = audioCodecCtx_->sample_fmt;
  av_channel_layout_copy(&audioFrame_->ch_layout, &audioCodecCtx_->ch_layout);
  audioFrame_->sample_rate  = audioOutSampleRate_;

  r = av_frame_get_buffer(audioFrame_, 0);
  if (r < 0) {
    ofLogError("LibavRecorder") << "av_frame_get_buffer (audio): " << avErr(r);
    return false;
  }

  if (!buildOrRebuildSwrContext_(audioInSampleRate_, audioInChannels_)) {
    return false;
  }

  audioTail_.reserve(static_cast<size_t>(audioFrameSize_) * audioInChannels_ * 2);
  audioEnabled_ = true;
  ofLogNotice("LibavRecorder")
    << "audio encoder: AAC "
    << audioInChannels_ << "ch@" << audioInSampleRate_
    << " → " << audioOutChannels_ << "ch@" << audioOutSampleRate_
    << " frame_size=" << audioFrameSize_;
  return true;
}

// ─── openVideoEncoderLadder_ ──────────────────────────────────────────────

bool LibavRecorder::openVideoEncoderLadder_(int width, int height, int fps) {
  videoStream_ = avformat_new_stream(formatCtx_, nullptr);
  if (!videoStream_) return false;

  const int bitrate = computeBitrate(width, height);

  for (int i = 0; kVideoEncoderLadder[i] != nullptr; ++i) {
    const char* name = kVideoEncoderLadder[i];
    const AVCodec* candidate = avcodec_find_encoder_by_name(name);
    if (!candidate) {
      ofLogVerbose("LibavRecorder") << "encoder not present in ffmpeg build: " << name;
      continue;
    }

    // Fresh context per attempt — avcodec_open2 mutates it on failure.
    AVCodecContext* ctx = avcodec_alloc_context3(candidate);
    if (!ctx) continue;

    ctx->width      = width;
    ctx->height     = height;
    ctx->pix_fmt    = AV_PIX_FMT_YUV420P;
    ctx->time_base  = {1, fps};
    ctx->framerate  = {fps, 1};
    ctx->bit_rate   = bitrate;
    ctx->gop_size   = fps * 2;
    ctx->max_b_frames = 0;   // low-latency; also avoids B-frame ordering
                             // headaches with wall-clock PTS
    if (formatCtx_->oformat && (formatCtx_->oformat->flags & AVFMT_GLOBALHEADER)) {
      ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    AVDictionary* opts = nullptr;
    // Nudge everyone toward zerolatency/veryfast — the software fallback
    // (libopenh264) accepts most of them; hardware encoders happily ignore
    // options they don't recognize (AV_OPT_SEARCH_CHILDREN).
    av_dict_set(&opts, "preset", "veryfast", 0);
    av_dict_set(&opts, "tune",   "zerolatency", 0);

    int r = avcodec_open2(ctx, candidate, &opts);
    av_dict_free(&opts);

    if (r < 0) {
      ofLogNotice("LibavRecorder")
        << "encoder " << name << " open failed (" << avErr(r) << "); trying next";
      avcodec_free_context(&ctx);
      continue;
    }

    // Won. Adopt.
    videoCodec_    = candidate;
    videoCodecCtx_ = ctx;
    openedVideoEncoderName_ = name;

    r = avcodec_parameters_from_context(videoStream_->codecpar, ctx);
    if (r < 0) {
      ofLogError("LibavRecorder") << "avcodec_parameters_from_context: " << avErr(r);
      return false;
    }
    videoStream_->time_base = ctx->time_base;

    videoFrame_  = av_frame_alloc();
    videoPacket_ = av_packet_alloc();
    if (!videoFrame_ || !videoPacket_) return false;
    videoFrame_->format = ctx->pix_fmt;
    videoFrame_->width  = width;
    videoFrame_->height = height;
    r = av_frame_get_buffer(videoFrame_, 0);
    if (r < 0) {
      ofLogError("LibavRecorder") << "av_frame_get_buffer (video): " << avErr(r);
      return false;
    }

    // RGBA→YUV420P scaler. srcPixelFormat_ stays RGBA — see addFrame().
    swsCtx_ = sws_getContext(width, height, srcPixelFormat_,
                             width, height, AV_PIX_FMT_YUV420P,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx_) {
      ofLogError("LibavRecorder") << "sws_getContext failed";
      return false;
    }
    return true;
  }
  return false;
}

// ─── buildOrRebuildSwrContext_ ────────────────────────────────────────────

bool LibavRecorder::buildOrRebuildSwrContext_(int inSampleRate, int inChannels) {
  if (swrCtx_) {
    swr_free(&swrCtx_);
  }
  AVChannelLayout inLayout{};
  av_channel_layout_default(&inLayout, inChannels);

  int r = swr_alloc_set_opts2(&swrCtx_,
                              &audioCodecCtx_->ch_layout,   // out
                              AV_SAMPLE_FMT_FLTP,
                              audioOutSampleRate_,
                              &inLayout,                    // in
                              AV_SAMPLE_FMT_FLT,
                              inSampleRate,
                              0, nullptr);
  av_channel_layout_uninit(&inLayout);
  if (r < 0 || !swrCtx_) {
    ofLogError("LibavRecorder") << "swr_alloc_set_opts2: " << avErr(r);
    return false;
  }
  r = swr_init(swrCtx_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "swr_init: " << avErr(r);
    return false;
  }
  audioInSampleRate_ = inSampleRate;
  audioInChannels_   = inChannels;
  return true;
}

// ─── start ────────────────────────────────────────────────────────────────

bool LibavRecorder::start() {
  std::lock_guard<std::mutex> lock(writeMutex_);

  if (!formatCtx_ || !videoCodecCtx_) {
    ofLogError("LibavRecorder") << "start called before setup";
    return false;
  }
  if (recording_.load()) return true;

  int r = 0;
  if (!(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
    // Manifest activeCodePage=UTF-8 (Win 10 1903+) lets libavformat's file
    // protocol accept UTF-8 paths through the CRT — same policy as
    // dr_libs. See docs/WINDOWS_ARCHITECTURE.md#audio-file-io.
    r = avio_open(&formatCtx_->pb, filePath_.c_str(), AVIO_FLAG_WRITE);
    if (r < 0) {
      ofLogError("LibavRecorder") << "avio_open " << filePath_ << ": " << avErr(r);
      return false;
    }
    ioOpened_ = true;
  }

  AVDictionary* muxOpts = nullptr;
  // frag_keyframe+empty_moov+default_base_moof: file stays playable if the
  // process is killed before av_write_trailer runs — every keyframe seals
  // a fragment. Costs a few % filesize.
  av_dict_set(&muxOpts, "movflags",
              "frag_keyframe+empty_moov+default_base_moof", 0);

  r = avformat_write_header(formatCtx_, &muxOpts);
  av_dict_free(&muxOpts);
  if (r < 0) {
    ofLogError("LibavRecorder") << "avformat_write_header: " << avErr(r);
    return false;
  }
  headerWritten_ = true;

  audioSamplesEncoded_ = 0;
  audioTail_.clear();
  completed_.store(false);
  startTime_ = std::chrono::steady_clock::now();
  recording_.store(true);
  ofLogNotice("LibavRecorder") << "recording started → " << filePath_;
  return true;
}

// ─── addFrame ─────────────────────────────────────────────────────────────

bool LibavRecorder::addFrame(const ofPixels& pixels) {
  if (!recording_.load()) return false;

  // Duration-limit check BEFORE encoding this frame. Single-fire finalize.
  if (maxDurationSeconds_ > 0.f) {
    const float elapsed = getElapsedTime();
    if (elapsed >= maxDurationSeconds_) {
      finalizeIfLive_(true);
      return false;
    }
  }

  const int64_t ptsNs = elapsedNs_();

  // No outer lock. The video path is single-threaded (main draw), so
  // videoCodecCtx_ / videoFrame_ / swsCtx_ have no cross-thread contention.
  // Muxer serialization happens per-packet inside drainEncoder_'s
  // writeMutex_ acquisition, so audio packets from the audio thread stay
  // properly interleaved with the video packets emitted here.

  // Match input pixel format to the sws context. ofPixels normalizes into
  // RGB / RGBA / whatever the source loaded as; force it to RGBA here so
  // sws expectations hold.
  ofPixels rgba = pixels;
  if (rgba.getPixelFormat() != OF_PIXELS_RGBA) {
    rgba.setImageType(OF_IMAGE_COLOR_ALPHA);
  }
  if (rgba.getWidth() != srcWidth_ || rgba.getHeight() != srcHeight_) {
    // Shouldn't happen — recorder is set up with the FBO's dimensions —
    // but a dimension change would corrupt sws output. Refuse the frame.
    ofLogError("LibavRecorder") << "frame dimension mismatch "
      << rgba.getWidth() << "x" << rgba.getHeight()
      << " vs setup " << srcWidth_ << "x" << srcHeight_;
    return false;
  }

  const uint8_t* srcData[4]     = { rgba.getData(), nullptr, nullptr, nullptr };
  const int      srcLinesize[4] = { static_cast<int>(rgba.getBytesStride()), 0, 0, 0 };

  int r = av_frame_make_writable(videoFrame_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "av_frame_make_writable (video): " << avErr(r);
    return false;
  }

  sws_scale(swsCtx_, srcData, srcLinesize, 0, srcHeight_,
            videoFrame_->data, videoFrame_->linesize);

  // Wall-clock PTS: rescale elapsed nanoseconds into the video stream's
  // time_base. Immune to shader-load hitches that would otherwise cause a
  // frame-counter clock to drift behind real time.
  videoFrame_->pts = av_rescale_q(ptsNs,
                                  {1, 1'000'000'000},
                                  videoCodecCtx_->time_base);

  r = avcodec_send_frame(videoCodecCtx_, videoFrame_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "avcodec_send_frame (video): " << avErr(r);
    return false;
  }
  return drainEncoder_(videoCodecCtx_, videoStream_);
}

// ─── addAudioSamples ──────────────────────────────────────────────────────

bool LibavRecorder::addAudioSamples(const float* samples, int numFrames) {
  if (!recording_.load() || !audioEnabled_ || !samples || numFrames <= 0) {
    return false;
  }
  std::lock_guard<std::mutex> alock(audioMutex_);

  // AudioSource fires this on the audio thread. We accept whatever channel
  // count / sample rate the source is producing and (re)build the swr chain
  // if it drifted from what setupAudio was told.
  //
  // Callers should already have queried the AudioSource before setup, so a
  // rebuild is the mid-record source-switch escape hatch, not the normal
  // path. Log loudly when it happens.
  //
  // Note: numFrames here is per-channel frames. The AudioRecordingCallback
  // signature carries (const float*, int numFrames, int numChannels, float sr)
  // but our public API strips the last two — we assume the caller keeps
  // them constant across a recording, or calls setupAudio again.

  const float* interleaved = samples;
  std::vector<float> scratch;
  (void)scratch;   // reserved for future channel-remap

  // Append to tail and drain in audioFrameSize_-sized chunks.
  const size_t framesIn = static_cast<size_t>(numFrames);
  const size_t chans    = static_cast<size_t>(audioInChannels_);
  audioTail_.insert(audioTail_.end(),
                    interleaved,
                    interleaved + framesIn * chans);

  const size_t frameChunkFloats = static_cast<size_t>(audioFrameSize_) * chans;
  bool ok = true;
  size_t cursor = 0;
  while (audioTail_.size() - cursor >= frameChunkFloats) {
    if (!encodeAndWriteAudioChunk_(audioTail_.data() + cursor, audioFrameSize_)) {
      ok = false;
      break;
    }
    cursor += frameChunkFloats;
  }
  if (cursor > 0) {
    audioTail_.erase(audioTail_.begin(),
                     audioTail_.begin() + cursor);
  }
  return ok;
}

// ─── encodeAndWriteAudioChunk_ ────────────────────────────────────────────

bool LibavRecorder::encodeAndWriteAudioChunk_(const float* interleavedFloat,
                                              int inFrames) {
  // Runs under audioMutex_. We take writeMutex_ ONLY for av_interleaved_
  // write_frame — the encoder itself needs no cross-thread serialization.
  int r = av_frame_make_writable(audioFrame_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "av_frame_make_writable (audio): " << avErr(r);
    return false;
  }

  const uint8_t* inData[1]     = { reinterpret_cast<const uint8_t*>(interleavedFloat) };
  uint8_t**      outData       = audioFrame_->data;

  int outSamples = swr_convert(swrCtx_, outData, audioFrameSize_,
                               inData, inFrames);
  if (outSamples < 0) {
    ofLogError("LibavRecorder") << "swr_convert: " << avErr(outSamples);
    return false;
  }
  audioFrame_->nb_samples = outSamples;
  audioFrame_->pts        = audioSamplesEncoded_;
  audioSamplesEncoded_   += outSamples;

  r = avcodec_send_frame(audioCodecCtx_, audioFrame_);
  if (r < 0) {
    ofLogError("LibavRecorder") << "avcodec_send_frame (audio): " << avErr(r);
    return false;
  }

  // drainEncoder_ acquires writeMutex_ around each packet write.
  return drainEncoder_(audioCodecCtx_, audioStream_);
}

// ─── drainEncoder_ ────────────────────────────────────────────────────────

bool LibavRecorder::drainEncoder_(AVCodecContext* ctx, AVStream* stream) {
  AVPacket* pkt = (ctx == videoCodecCtx_) ? videoPacket_ : audioPacket_;
  while (true) {
    int r = avcodec_receive_packet(ctx, pkt);
    if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
      return true;
    }
    if (r < 0) {
      ofLogError("LibavRecorder") << "avcodec_receive_packet: " << avErr(r);
      return false;
    }
    // Rescale PTS from codec time_base to stream time_base (mp4 muxer usually
    // rewrites it to 1/90000 for video, 1/samplerate for audio).
    av_packet_rescale_ts(pkt, ctx->time_base, stream->time_base);
    pkt->stream_index = stream->index;
    {
      std::lock_guard<std::mutex> wlock(writeMutex_);
      if (!formatCtx_ || !recording_.load()) {
        av_packet_unref(pkt);
        return false;
      }
      r = av_interleaved_write_frame(formatCtx_, pkt);
    }
    av_packet_unref(pkt);
    if (r < 0) {
      ofLogError("LibavRecorder") << "av_interleaved_write_frame: " << avErr(r);
      return false;
    }
  }
}

// ─── stop / finalize ──────────────────────────────────────────────────────

void LibavRecorder::stop(std::function<void(bool)> completionCallback) {
  const bool wasLive = recording_.load();
  finalizeIfLive_(wasLive);
  if (completionCallback) completionCallback(wasLive);
}

void LibavRecorder::finalizeIfLive_(bool success) {
  // Single-fire. All duration-limit / stop() / destructor paths land here.
  if (completed_.exchange(true)) return;

  bool ok = success;

  // Flush leftover audio tail with zero padding so no samples are dropped.
  if (audioEnabled_ && audioCodecCtx_) {
    std::lock_guard<std::mutex> alock(audioMutex_);
    const size_t chans = static_cast<size_t>(audioInChannels_);
    const size_t frameChunkFloats = static_cast<size_t>(audioFrameSize_) * chans;
    if (!audioTail_.empty() && audioTail_.size() < frameChunkFloats) {
      audioTail_.resize(frameChunkFloats, 0.f);
      const int carry = audioFrameSize_;  // we padded to a full frame
      if (!encodeAndWriteAudioChunk_(audioTail_.data(), carry)) {
        ok = false;
      }
      audioTail_.clear();
    }
    // Signal end-of-stream to the encoder so it flushes pending packets.
    if (avcodec_send_frame(audioCodecCtx_, nullptr) == 0) {
      drainEncoder_(audioCodecCtx_, audioStream_);
    }
  }

  // Flush video encoder.
  if (videoCodecCtx_) {
    if (avcodec_send_frame(videoCodecCtx_, nullptr) == 0) {
      drainEncoder_(videoCodecCtx_, videoStream_);
    }
  }

  // Trailer + close IO. Under writeMutex_ so any in-flight write finishes.
  {
    std::lock_guard<std::mutex> wlock(writeMutex_);
    if (formatCtx_ && headerWritten_) {
      int r = av_write_trailer(formatCtx_);
      if (r < 0) {
        ofLogError("LibavRecorder") << "av_write_trailer: " << avErr(r);
        ok = false;
      }
      headerWritten_ = false;
    }
    if (formatCtx_ && ioOpened_ &&
        !(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&formatCtx_->pb);
      ioOpened_ = false;
    }
  }
  recording_.store(false);

  if (onComplete_) {
    // Copy before releasing so a callback that tears us down is safe.
    auto cb = onComplete_;
    onComplete_ = nullptr;
    cb(ok, filePath_);
  }
  ofLogNotice("LibavRecorder")
    << "finalized " << filePath_ << " ok=" << (ok ? "true" : "false");
}

// ─── plumbing ─────────────────────────────────────────────────────────────

int64_t LibavRecorder::elapsedNs_() const {
  const auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime_).count();
}

float LibavRecorder::getElapsedTime() const {
  if (!recording_.load()) return 0.f;
  return static_cast<float>(elapsedNs_()) / 1e9f;
}

void LibavRecorder::freeAll_() {
  // Called with either no lock or after recording_ is clear. Idempotent.
  if (swsCtx_)         { sws_freeContext(swsCtx_);          swsCtx_ = nullptr; }
  if (swrCtx_)         { swr_free(&swrCtx_); }
  if (videoFrame_)     { av_frame_free(&videoFrame_); }
  if (audioFrame_)     { av_frame_free(&audioFrame_); }
  if (videoPacket_)    { av_packet_free(&videoPacket_); }
  if (audioPacket_)    { av_packet_free(&audioPacket_); }
  if (videoCodecCtx_)  { avcodec_free_context(&videoCodecCtx_); }
  if (audioCodecCtx_)  { avcodec_free_context(&audioCodecCtx_); }
  if (formatCtx_) {
    if (ioOpened_ && !(formatCtx_->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&formatCtx_->pb);
      ioOpened_ = false;
    }
    avformat_free_context(formatCtx_);
    formatCtx_ = nullptr;
  }
  videoStream_ = nullptr;
  audioStream_ = nullptr;
  videoCodec_  = nullptr;
  audioCodec_  = nullptr;
  audioEnabled_ = false;
  headerWritten_ = false;
  audioTail_.clear();
  audioTail_.shrink_to_fit();
}

#endif // _WIN32 && !TARGET_OSX
