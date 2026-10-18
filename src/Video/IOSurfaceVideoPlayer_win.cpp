// IOSurfaceVideoPlayer_win.cpp
//
// Windows implementation of IOSurfaceVideoPlayer (the macOS one is
// AVFoundation, IOSurfaceVideoPlayer.mm), decoding with libav.
//
//   decode thread: demux → decode (software, multithreaded) → sws_scale to
//                  BGRA → a short queue of frames ahead of the playhead
//   update():      (engine thread) picks the frame due at the playback clock
//                  and uploads it into a GL_TEXTURE_2D, wrapped by an
//                  ofTexture for draw()
//
// The playback clock is wall time scaled by speed. setPosition() seeks
// (frame-accurately: decoding forward from the preceding keyframe). Reverse
// speeds are served by seeking to the clock as it runs backwards — slow, but
// they're only used by the (currently hidden) boomerang option. Audio isn't
// played; FileSource keeps videos muted.

#ifndef TARGET_OSX

#include "IOSurfaceVideoPlayer.hpp"

#include "ofGLUtils.h"
#include "ofLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace nottawa {

namespace {

constexpr size_t kQueueFrames = 4;  // decoded frames kept ahead of the playhead

double nowSeconds() {
  using namespace std::chrono;
  return duration<double>(steady_clock::now().time_since_epoch()).count();
}

struct DecodedFrame {
  double pts = 0;               // seconds
  std::vector<uint8_t> bgra;    // width * height * 4
};

} // namespace

struct IOSurfaceVideoPlayer::Impl {
  // ---- libav (decode thread, after load) ----
  AVFormatContext* format = nullptr;
  AVCodecContext*  codec  = nullptr;
  SwsContext*      sws    = nullptr;
  int              stream = -1;
  double           timeBase = 0;   // seconds per pts unit
  double           startTime = 0;  // stream start, seconds

  // ---- metadata ----
  std::string path;
  int    width = 0, height = 0;
  float  duration = 0;
  float  fps = 30;
  bool   loaded = false;

  // ---- decode thread / queue (guarded by m) ----
  std::thread             thread;
  std::mutex              m;
  std::condition_variable cv;
  std::deque<DecodedFrame> queue;
  std::vector<std::vector<uint8_t>> spare;  // recycled frame buffers
  bool   quit = false;
  bool   eof = false;
  bool   seekRequested = false;
  double seekTarget = 0;
  uint64_t seekGeneration = 0;   // bumped per seek; frames from older seeks are dropped

  // ---- playback clock (guarded by m) ----
  bool   playing = false;
  float  speed = 1;
  double clockBase = 0;  // media seconds at wallBase
  double wallBase = 0;

  // ---- engine-thread state ----
  ofLoopType loopState = OF_LOOP_NORMAL;
  std::atomic<bool> movieDone{false};
  bool   frameNew = false;
  double shownPts = -1;
  double lastReverseSeek = -1;
  GLuint texture = 0;
  ofTexture videoTexture;

  double mediaTimeLocked() const {
    double t = clockBase;
    if (playing) t += (nowSeconds() - wallBase) * speed;
    if (t < 0) t = 0;
    if (duration > 0 && t > duration) t = duration;
    return t;
  }
  void rebaseLocked() {
    clockBase = mediaTimeLocked();
    wallBase = nowSeconds();
  }
  void requestSeekLocked(double target) {
    seekRequested = true;
    seekTarget = target;
    ++seekGeneration;
    for (auto& f : queue) spare.push_back(std::move(f.bgra));
    queue.clear();
    eof = false;
    cv.notify_all();
  }

  void decodeLoop();
  bool decodeUntilFrame(AVPacket* packet, AVFrame* frame, uint64_t generation, double dropBefore);
  void pushFrame(AVFrame* frame, uint64_t generation);
  void freeLibav();
};

// ---------------------------------------------------------------- decoding

void IOSurfaceVideoPlayer::Impl::pushFrame(AVFrame* frame, uint64_t generation) {
  int64_t ts = frame->best_effort_timestamp;
  if (ts == AV_NOPTS_VALUE) ts = frame->pts;
  const double pts = (ts == AV_NOPTS_VALUE ? 0 : ts * timeBase) - startTime;

  std::vector<uint8_t> buffer;
  {
    std::lock_guard<std::mutex> lk(m);
    if (!spare.empty()) { buffer = std::move(spare.back()); spare.pop_back(); }
  }
  buffer.resize(size_t(width) * height * 4);
  sws = sws_getCachedContext(sws, frame->width, frame->height, AVPixelFormat(frame->format),
                             width, height, AV_PIX_FMT_BGRA, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!sws) return;
  uint8_t* dst[1] = {buffer.data()};
  const int dstStride[1] = {width * 4};
  sws_scale(sws, frame->data, frame->linesize, 0, frame->height, dst, dstStride);

  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [&] { return quit || seekGeneration != generation || queue.size() < kQueueFrames; });
  if (quit || seekGeneration != generation) { spare.push_back(std::move(buffer)); return; }
  queue.push_back({pts, std::move(buffer)});
}

// Feeds packets until the decoder yields frames; pushes those at or after
// `dropBefore`. Returns false at end of stream (after draining).
bool IOSurfaceVideoPlayer::Impl::decodeUntilFrame(AVPacket* packet, AVFrame* frame, uint64_t generation,
                                                  double dropBefore) {
  const double halfFrame = 0.5 / std::max(1.0f, fps);
  for (;;) {
    const int got = avcodec_receive_frame(codec, frame);
    if (got == 0) {
      int64_t ts = frame->best_effort_timestamp;
      if (ts == AV_NOPTS_VALUE) ts = frame->pts;
      const double pts = (ts == AV_NOPTS_VALUE ? 0 : ts * timeBase) - startTime;
      if (pts + halfFrame >= dropBefore) pushFrame(frame, generation);
      av_frame_unref(frame);
      return true;
    }
    if (got == AVERROR_EOF) return false;
    if (got != AVERROR(EAGAIN)) return false;

    const int read = av_read_frame(format, packet);
    if (read < 0) {
      avcodec_send_packet(codec, nullptr);  // drain
      continue;
    }
    if (packet->stream_index == stream) avcodec_send_packet(codec, packet);
    av_packet_unref(packet);
  }
}

void IOSurfaceVideoPlayer::Impl::decodeLoop() {
  AVPacket* packet = av_packet_alloc();
  AVFrame*  frame  = av_frame_alloc();
  double dropBefore = 0;
  for (;;) {
    uint64_t generation;
    {
      std::unique_lock<std::mutex> lk(m);
      cv.wait(lk, [&] { return quit || seekRequested || (!eof && queue.size() < kQueueFrames); });
      if (quit) break;
      generation = seekGeneration;
      if (seekRequested) {
        seekRequested = false;
        const double target = seekTarget;
        lk.unlock();
        const int64_t ts = int64_t((target + startTime) / timeBase);
        av_seek_frame(format, stream, ts, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codec);
        dropBefore = target;
        continue;
      }
    }
    if (!decodeUntilFrame(packet, frame, generation, dropBefore)) {
      std::lock_guard<std::mutex> lk(m);
      if (seekGeneration == generation) eof = true;
    }
  }
  av_frame_free(&frame);
  av_packet_free(&packet);
}

void IOSurfaceVideoPlayer::Impl::freeLibav() {
  if (sws)    { sws_freeContext(sws); sws = nullptr; }
  if (codec)  avcodec_free_context(&codec);
  if (format) avformat_close_input(&format);
  stream = -1;
}

// ---------------------------------------------------------------- lifecycle

IOSurfaceVideoPlayer::IOSurfaceVideoPlayer() : impl_(std::make_unique<Impl>()) {}

IOSurfaceVideoPlayer::~IOSurfaceVideoPlayer() { close(); }

bool IOSurfaceVideoPlayer::load(const std::string& path) {
  // Same file already loaded: keep it (FileSource re-runs setup() on
  // resolution changes).
  if (impl_->loaded && impl_->path == path) return true;
  close();
  Impl& d = *impl_;

  if (avformat_open_input(&d.format, path.c_str(), nullptr, nullptr) < 0) {
    ofLogError("IOSurfaceVideoPlayer") << "can't open " << path;
    return false;
  }
  if (avformat_find_stream_info(d.format, nullptr) < 0) {
    ofLogError("IOSurfaceVideoPlayer") << "no stream info in " << path;
    d.freeLibav();
    return false;
  }
  const AVCodec* decoder = nullptr;
  d.stream = av_find_best_stream(d.format, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
  if (d.stream < 0 || !decoder) {
    ofLogError("IOSurfaceVideoPlayer") << "no decodable video stream in " << path;
    d.freeLibav();
    return false;
  }
  AVStream* st = d.format->streams[d.stream];
  d.codec = avcodec_alloc_context3(decoder);
  avcodec_parameters_to_context(d.codec, st->codecpar);
  d.codec->thread_count = 0;  // auto
  d.codec->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
  if (avcodec_open2(d.codec, decoder, nullptr) < 0) {
    ofLogError("IOSurfaceVideoPlayer") << "can't open decoder for " << path;
    d.freeLibav();
    return false;
  }

  d.width = d.codec->width;
  d.height = d.codec->height;
  d.timeBase = av_q2d(st->time_base);
  d.startTime = st->start_time != AV_NOPTS_VALUE ? st->start_time * d.timeBase : 0;
  if (st->duration != AV_NOPTS_VALUE) d.duration = float(st->duration * d.timeBase);
  else if (d.format->duration != AV_NOPTS_VALUE) d.duration = float(d.format->duration / double(AV_TIME_BASE));
  const AVRational rate = st->avg_frame_rate.num ? st->avg_frame_rate : st->r_frame_rate;
  d.fps = rate.num && rate.den ? float(av_q2d(rate)) : 30.0f;
  if (d.width <= 0 || d.height <= 0) {
    d.freeLibav();
    return false;
  }

  glGenTextures(1, &d.texture);
  glBindTexture(GL_TEXTURE_2D, d.texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, d.width, d.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  d.videoTexture.allocate(d.width, d.height, GL_RGBA);
  d.videoTexture.setUseExternalTextureID(d.texture);

  d.path = path;
  d.loaded = true;
  d.quit = false;
  d.eof = false;
  d.playing = false;
  d.speed = 1;
  d.clockBase = 0;
  d.wallBase = nowSeconds();
  d.shownPts = -1;
  d.movieDone = false;
  d.frameNew = false;
  d.thread = std::thread([&d] { d.decodeLoop(); });
  return true;
}

bool IOSurfaceVideoPlayer::isLoaded() const { return impl_->loaded; }

void IOSurfaceVideoPlayer::close() {
  Impl& d = *impl_;
  if (d.thread.joinable()) {
    {
      std::lock_guard<std::mutex> lk(d.m);
      d.quit = true;
    }
    d.cv.notify_all();
    d.thread.join();
  }
  d.freeLibav();
  d.queue.clear();
  d.spare.clear();
  if (d.texture) {
    // The ofTexture only references it (external ID), so free it here.
    glDeleteTextures(1, &d.texture);
    d.texture = 0;
  }
  d.videoTexture.clear();
  d.loaded = false;
  d.frameNew = false;
  d.path.clear();
}

void IOSurfaceVideoPlayer::setVolume(float) {}

float IOSurfaceVideoPlayer::getDuration() const { return impl_->duration; }
int   IOSurfaceVideoPlayer::getTotalNumFrames() const {
  return std::max(1, int(std::round(impl_->fps * impl_->duration)));
}
float IOSurfaceVideoPlayer::getWidth() const  { return float(impl_->width); }
float IOSurfaceVideoPlayer::getHeight() const { return float(impl_->height); }

// ---------------------------------------------------------------- playback

void IOSurfaceVideoPlayer::play() {
  std::lock_guard<std::mutex> lk(impl_->m);
  if (impl_->playing) return;
  impl_->rebaseLocked();
  impl_->playing = true;
}

void IOSurfaceVideoPlayer::stop() {
  {
    std::lock_guard<std::mutex> lk(impl_->m);
    impl_->rebaseLocked();
    impl_->playing = false;
  }
  setPosition(0);
}

void IOSurfaceVideoPlayer::setPaused(bool paused) {
  std::lock_guard<std::mutex> lk(impl_->m);
  impl_->rebaseLocked();
  impl_->playing = !paused;
}

bool IOSurfaceVideoPlayer::isPlaying() const {
  std::lock_guard<std::mutex> lk(impl_->m);
  return impl_->loaded && impl_->playing && impl_->speed != 0;
}

void IOSurfaceVideoPlayer::setPosition(float pct) {
  Impl& d = *impl_;
  if (!d.loaded) return;
  pct = std::min(1.0f, std::max(0.0f, pct));
  std::lock_guard<std::mutex> lk(d.m);
  d.clockBase = pct * d.duration;
  d.wallBase = nowSeconds();
  d.requestSeekLocked(d.clockBase);
  d.movieDone = false;
}

float IOSurfaceVideoPlayer::getPosition() const {
  Impl& d = *impl_;
  if (!d.loaded || d.duration <= 0) return 0;
  std::lock_guard<std::mutex> lk(d.m);
  return float(d.mediaTimeLocked() / d.duration);
}

void IOSurfaceVideoPlayer::setSpeed(float speed) {
  std::lock_guard<std::mutex> lk(impl_->m);
  impl_->rebaseLocked();
  impl_->speed = speed;
}

float IOSurfaceVideoPlayer::getSpeed() const {
  std::lock_guard<std::mutex> lk(impl_->m);
  return impl_->speed;
}

void IOSurfaceVideoPlayer::setLoopState(ofLoopType state) { impl_->loopState = state; }

// ---------------------------------------------------------------- frames

void IOSurfaceVideoPlayer::update() {
  Impl& d = *impl_;
  d.frameNew = false;
  if (!d.loaded) return;

  const double frameDuration = 1.0 / std::max(1.0f, d.fps);
  DecodedFrame due;
  bool haveFrame = false, atEnd = false;
  {
    std::lock_guard<std::mutex> lk(d.m);
    const double t = d.mediaTimeLocked();
    if (d.speed < 0 && d.playing) {
      // Reverse: decode the frame under the backwards-running clock.
      if (std::fabs(t - d.shownPts) >= frameDuration && std::fabs(t - d.lastReverseSeek) >= frameDuration) {
        d.lastReverseSeek = t;
        d.requestSeekLocked(t);
      }
    }
    // Skip frames that are already late, then take the one that's due.
    while (d.queue.size() > 1 && d.queue[1].pts <= t + frameDuration * 0.5) {
      d.spare.push_back(std::move(d.queue.front().bgra));
      d.queue.pop_front();
    }
    if (!d.queue.empty() && d.queue.front().pts <= t + frameDuration * 0.5) {
      due = std::move(d.queue.front());
      d.queue.pop_front();
      haveFrame = true;
      d.cv.notify_all();
    }
    atEnd = d.eof && d.queue.empty() && d.speed >= 0 && t >= d.shownPts;
  }

  if (haveFrame) {
    glBindTexture(GL_TEXTURE_2D, d.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, d.width, d.height, GL_BGRA, GL_UNSIGNED_BYTE, due.bgra.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    d.shownPts = due.pts;
    d.frameNew = true;
    std::lock_guard<std::mutex> lk(d.m);
    d.spare.push_back(std::move(due.bgra));
  }

  if (atEnd && !d.movieDone) {
    d.movieDone = true;
    if (d.loopState == OF_LOOP_NORMAL) {
      setPosition(0);
      play();
    }
  }
}

bool IOSurfaceVideoPlayer::isFrameNew() const { return impl_->frameNew; }
bool IOSurfaceVideoPlayer::getIsMovieDone() const { return impl_->movieDone.load(); }

void IOSurfaceVideoPlayer::draw(float x, float y, float w, float h) {
  if (impl_->videoTexture.isAllocated() && impl_->shownPts >= 0) impl_->videoTexture.draw(x, y, w, h);
}

} // namespace nottawa

#endif // !TARGET_OSX
