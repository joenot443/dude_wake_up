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
// they're only used by the (currently hidden) boomerang option.
//
// Audio (AudioTrack, below) is started the first time a volume above zero is
// set — FileSource starts videos muted. It demuxes the file separately, so
// its read-ahead doesn't depend on the video queue, decodes about a second
// ahead to float stereo, and plays through a WASAPI shared-mode stream,
// keeping each sample at the playback clock (skipping or padding small
// drift, re-seeking on jumps such as seeks and loops). It plays only at 1x
// forward speed; other speeds are silent.

#ifndef TARGET_OSX

#include "IOSurfaceVideoPlayer.hpp"

#include "ofGLUtils.h"
#include "ofLog.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <audioclient.h>
#include <mmdeviceapi.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
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

// ---------------------------------------------------------------- audio

// A video's audio, played in step with the player's clock.
class AudioTrack {
public:
  // `clock` returns the media time and whether audio should be heard now.
  using Clock = std::function<double(bool* audible)>;

  static std::unique_ptr<AudioTrack> open(const std::string& path, double origin, Clock clock) {
    std::unique_ptr<AudioTrack> track(new AudioTrack(origin, std::move(clock)));
    if (!track->openDecoder(path)) return nullptr;
    track->decodeThread_ = std::thread([t = track.get()] { t->decodeLoop(); });
    track->renderThread_ = std::thread([t = track.get()] { t->renderLoop(); });
    return track;
  }

  ~AudioTrack() {
    quit_ = true;
    if (event_) SetEvent(event_);
    if (decodeThread_.joinable()) decodeThread_.join();
    if (renderThread_.joinable()) renderThread_.join();
    if (event_) CloseHandle(event_);
    if (swr_) swr_free(&swr_);
    if (codec_) avcodec_free_context(&codec_);
    if (format_) avformat_close_input(&format_);
  }

  void setVolume(float v) { volume_ = v; }

private:
  struct Chunk {
    double pts = 0;            // seconds of the first sample
    std::vector<float> data;   // interleaved stereo
    size_t pos = 0;            // samples consumed (not frames)
  };
  static constexpr double kReadAhead = 1.0;   // seconds decoded ahead of the clock
  static constexpr double kTolerance = 0.04;  // drift corrected by skipping/padding
  static constexpr double kJump = 0.5;        // drift that re-seeks instead

  AudioTrack(double origin, Clock clock) : origin_(origin), clock_(std::move(clock)) {}

  bool openDecoder(const std::string& path) {
    if (avformat_open_input(&format_, path.c_str(), nullptr, nullptr) < 0) return false;
    if (avformat_find_stream_info(format_, nullptr) < 0) return false;
    const AVCodec* decoder = nullptr;
    stream_ = av_find_best_stream(format_, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    if (stream_ < 0 || !decoder) return false;
    AVStream* st = format_->streams[stream_];
    codec_ = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codec_, st->codecpar);
    if (avcodec_open2(codec_, decoder, nullptr) < 0) return false;
    timeBase_ = av_q2d(st->time_base);
    rate_ = deviceRate();
    AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
    if (swr_alloc_set_opts2(&swr_, &stereo, AV_SAMPLE_FMT_FLT, rate_, &codec_->ch_layout, codec_->sample_fmt,
                            codec_->sample_rate, 0, nullptr) < 0 || swr_init(swr_) < 0)
      return false;
    event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    return true;
  }

  // The shared-mode mix rate, so the stream needs no resampling by Windows.
  static int deviceRate() {
    int rate = 48000;
    const bool com = SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    IMMDeviceEnumerator* devices = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* client = nullptr;
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&devices))) &&
        SUCCEEDED(devices->GetDefaultAudioEndpoint(eRender, eConsole, &device)) &&
        SUCCEEDED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&client)))) {
      WAVEFORMATEX* mix = nullptr;
      if (SUCCEEDED(client->GetMixFormat(&mix))) {
        rate = int(mix->nSamplesPerSec);
        CoTaskMemFree(mix);
      }
    }
    if (client) client->Release();
    if (device) device->Release();
    if (devices) devices->Release();
    if (com) CoUninitialize();
    return rate;
  }

  double bufferedEndLocked() const {
    if (chunks_.empty()) return -1;
    const Chunk& c = chunks_.back();
    return c.pts + double(c.data.size() / 2) / rate_;
  }

  void decodeLoop() {
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    double dropBefore = 0;
    while (!quit_) {
      bool audible = false;
      const double now = clock_(&audible);
      bool seek = false;
      double target = 0;
      {
        std::lock_guard<std::mutex> lk(m_);
        if (seekRequested_) {
          seekRequested_ = false;
          seek = true;
          target = seekTarget_;
          chunks_.clear();
        } else if (eof_ || bufferedEndLocked() > now + kReadAhead) {
          // Enough decoded (or nothing left): check again shortly.
        } else {
          target = -1;
        }
      }
      if (seek) {
        av_seek_frame(format_, stream_, int64_t((target + origin_) / timeBase_), AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codec_);
        dropBefore = target;
        std::lock_guard<std::mutex> lk(m_);
        eof_ = false;
        continue;
      }
      if (target != -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        continue;
      }
      if (av_read_frame(format_, packet) < 0) {
        std::lock_guard<std::mutex> lk(m_);
        eof_ = true;
        continue;
      }
      if (packet->stream_index == stream_ && avcodec_send_packet(codec_, packet) == 0) {
        while (avcodec_receive_frame(codec_, frame) == 0) {
          int64_t ts = frame->best_effort_timestamp;
          if (ts == AV_NOPTS_VALUE) ts = frame->pts;
          Chunk chunk;
          chunk.pts = (ts == AV_NOPTS_VALUE ? 0 : ts * timeBase_) - origin_;
          const int capacity = swr_get_out_samples(swr_, frame->nb_samples);
          chunk.data.resize(size_t(std::max(capacity, 0)) * 2);
          uint8_t* out = reinterpret_cast<uint8_t*>(chunk.data.data());
          const int n = swr_convert(swr_, &out, capacity, const_cast<const uint8_t**>(frame->extended_data),
                                    frame->nb_samples);
          av_frame_unref(frame);
          if (n <= 0) continue;
          chunk.data.resize(size_t(n) * 2);
          if (chunk.pts + double(n) / rate_ < dropBefore) continue;
          std::lock_guard<std::mutex> lk(m_);
          if (seekRequested_) continue;
          decodedEnd_ = chunk.pts + double(n) / rate_;
          chunks_.push_back(std::move(chunk));
        }
      }
      av_packet_unref(packet);
    }
    av_frame_free(&frame);
    av_packet_free(&packet);
  }

  // Fills `out` (frames of stereo float) for playback starting at `start`.
  void fill(float* out, UINT32 frames, double start, bool audible) {
    std::fill(out, out + size_t(frames) * 2, 0.0f);
    std::lock_guard<std::mutex> lk(m_);
    if (!audible) {
      // Keep the buffer from going stale behind a running (muted) clock.
      while (!chunks_.empty() && chunks_.front().pts + double(chunks_.front().data.size() / 2) / rate_ < start)
        chunks_.pop_front();
      return;
    }
    // Far off the clock (a seek or loop): re-seek rather than skip. With
    // nothing buffered, wait for the decoder unless the audio has ended and
    // the clock has since jumped back before its end.
    if (!seekRequested_) {
      bool jumped = false;
      if (!chunks_.empty()) jumped = std::fabs(chunks_.front().pts + double(chunks_.front().pos / 2) / rate_ - start) > kJump;
      else if (eof_) jumped = start < decodedEnd_ - kJump;
      if (jumped) {
        seekRequested_ = true;
        seekTarget_ = start;
        chunks_.clear();
        return;
      }
    }
    const float volume = volume_;
    UINT32 i = 0;
    while (i < frames && !chunks_.empty()) {
      Chunk& c = chunks_.front();
      const size_t left = (c.data.size() - c.pos) / 2;
      const double drift = c.pts + double(c.pos / 2) / rate_ - (start + double(i) / rate_);
      if (drift < -kTolerance) {        // behind: skip ahead
        c.pos += std::min(left, size_t(-drift * rate_)) * 2;
      } else if (drift > kTolerance) {  // ahead: leave silence until it's due
        i += std::min<UINT32>(frames - i, UINT32(drift * rate_));
        continue;
      } else {
        const size_t n = std::min<size_t>(frames - i, left);
        for (size_t k = 0; k < n * 2; ++k) out[size_t(i) * 2 + k] = c.data[c.pos + k] * volume;
        c.pos += n * 2;
        i += UINT32(n);
      }
      if (c.pos >= c.data.size()) chunks_.pop_front();
    }
  }

  void renderLoop() {
    const bool com = SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    while (!quit_) {
      IMMDeviceEnumerator* devices = nullptr;
      IMMDevice* device = nullptr;
      IAudioClient* client = nullptr;
      IAudioRenderClient* render = nullptr;
      WAVEFORMATEX format = {};
      format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
      format.nChannels = 2;
      format.nSamplesPerSec = DWORD(rate_);
      format.wBitsPerSample = 32;
      format.nBlockAlign = 8;
      format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
      UINT32 bufferFrames = 0;
      bool ok = SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&devices))) &&
                SUCCEEDED(devices->GetDefaultAudioEndpoint(eRender, eConsole, &device)) &&
                SUCCEEDED(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&client))) &&
                SUCCEEDED(client->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                             AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
                                                 AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                                             500000 /* 50 ms */, 0, &format, nullptr)) &&
                SUCCEEDED(client->SetEventHandle(event_)) && SUCCEEDED(client->GetBufferSize(&bufferFrames)) &&
                SUCCEEDED(client->GetService(IID_PPV_ARGS(&render))) && SUCCEEDED(client->Start());
      // Until the device goes away (unplugged, default changed): then reopen.
      while (ok && !quit_) {
        WaitForSingleObject(event_, 200);
        UINT32 padding = 0;
        if (FAILED(client->GetCurrentPadding(&padding))) break;
        const UINT32 frames = bufferFrames - padding;
        if (frames == 0) continue;
        BYTE* data = nullptr;
        if (FAILED(render->GetBuffer(frames, &data))) break;
        bool audible = false;
        const double now = clock_(&audible);
        fill(reinterpret_cast<float*>(data), frames, now + double(padding) / rate_, audible && volume_ > 0);
        render->ReleaseBuffer(frames, 0);
      }
      if (client) client->Stop();
      if (render) render->Release();
      if (client) client->Release();
      if (device) device->Release();
      if (devices) devices->Release();
      if (!quit_) std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    if (com) CoUninitialize();
  }

  double origin_;
  Clock  clock_;
  AVFormatContext* format_ = nullptr;
  AVCodecContext*  codec_ = nullptr;
  SwrContext*      swr_ = nullptr;
  int    stream_ = -1;
  double timeBase_ = 0;
  int    rate_ = 48000;
  HANDLE event_ = nullptr;
  std::atomic<bool>  quit_{false};
  std::atomic<float> volume_{0};
  std::thread decodeThread_, renderThread_;
  std::mutex  m_;  // guards everything below
  std::deque<Chunk> chunks_;
  bool   eof_ = false;
  double decodedEnd_ = 0;  // end of the last chunk decoded
  bool   seekRequested_ = false;
  double seekTarget_ = 0;
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
  std::unique_ptr<AudioTrack> audio;  // created once a volume above zero is set
  bool  noAudio = false;              // the file has no playable audio

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
  d.audio.reset();
  d.noAudio = false;
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

void IOSurfaceVideoPlayer::setVolume(float v) {
  Impl& d = *impl_;
  if (!d.loaded) return;
  if (!d.audio && v > 0 && !d.noAudio) {
    d.audio = AudioTrack::open(d.path, d.startTime, [&d](bool* audible) {
      std::lock_guard<std::mutex> lk(d.m);
      *audible = d.playing && std::fabs(d.speed - 1.0f) < 0.01f;
      return d.mediaTimeLocked();
    });
    d.noAudio = !d.audio;
    ofLogNotice("IOSurfaceVideoPlayer") << (d.audio ? "playing audio of " : "no audio in ") << d.path;
  }
  if (d.audio) d.audio->setVolume(v);
}

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
