//
//  IOSurfaceVideoPlayer.hpp
//
//  Drop-in replacement for the subset of ofVideoPlayer that FileSource uses.
//  Pipes AVFoundation's CVPixelBuffer directly to a GL_TEXTURE_2D via the
//  CGLTexImageIOSurface2D path — no CPU pixel decode, no CPU→GPU upload.
//
//  API mirrors ofVideoPlayer so call sites in FileSource / LibrarySource /
//  PlaylistSource / nottawa_bridge stay unchanged.
//

#ifndef IOSurfaceVideoPlayer_hpp
#define IOSurfaceVideoPlayer_hpp

#include "ofTexture.h"
#include "ofConstants.h"     // for ofPixelFormat
#include "ofVideoBaseTypes.h" // for ofLoopType
#include <memory>
#include <string>

namespace nottawa {

class IOSurfaceVideoPlayer {
public:
  IOSurfaceVideoPlayer();
  ~IOSurfaceVideoPlayer();

  // Non-copyable. Holds Obj-C runtime state and a GL texture.
  IOSurfaceVideoPlayer(const IOSurfaceVideoPlayer&) = delete;
  IOSurfaceVideoPlayer& operator=(const IOSurfaceVideoPlayer&) = delete;

  // ---- Lifecycle / metadata ----------------------------------------------
  bool load(const std::string& path);
  bool isLoaded() const;
  void close();
  // No-op — internally we always request kCVPixelFormatType_32BGRA for the
  // IOSurface, then bind it as a GL_RGBA texture. Kept for API parity.
  void setPixelFormat(ofPixelFormat /*fmt*/) {}
  void setVolume(float v);

  float getDuration() const;
  int   getTotalNumFrames() const;
  float getWidth() const;
  float getHeight() const;

  // ---- Playback ----------------------------------------------------------
  void  play();
  void  stop();
  void  setPaused(bool paused);
  bool  isPlaying() const;

  void  setPosition(float pct);   // 0..1 — matches ofVideoPlayer semantics
  float getPosition() const;      // 0..1

  void  setSpeed(float speed);
  float getSpeed() const;

  void  setLoopState(ofLoopType state);

  // ---- Frame state -------------------------------------------------------
  // Pulls the latest pixel buffer from AVPlayerItemVideoOutput and rebinds
  // it to our GL texture. Must be called on the engine thread (GL context).
  void  update();
  bool  isFrameNew() const;
  bool  getIsMovieDone() const;

  // ---- Rendering ---------------------------------------------------------
  // Draws the current bound texture into the active draw target.
  void  draw(float x, float y, float w, float h);

#ifndef TARGET_OSX
  // ---- Frame cache (Windows) ---------------------------------------------
  // After load, a background thread decodes the whole clip into RAM (as
  // YUV 4:2:0) if it fits the shared budget; once complete, playback in any
  // direction, scrubbing and looping read from it instead of the decoder.
  struct CacheStatus {
    bool enabled = true;      // the runtime switch (Use frame cache)
    bool complete = false;    // every frame is cached and in use
    bool overBudget = false;  // the clip didn't fit; it streams
    int  frames = 0;          // frames cached so far
    int  total = 0;           // frames in the clip
  };
  CacheStatus cacheStatus() const;
  void setCacheEnabled(bool on);
  // The app-wide settings (performance.frameCache / frameCacheBudget): whether
  // clips cache as they load, and the RAM all caches share (0: a quarter of
  // physical memory). Clips already loaded keep theirs.
  static void setCacheSettings(bool enabled, uint64_t budgetBytes);
#endif

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace nottawa

// Hoist into the global namespace so existing call sites that say
// `IOSurfaceVideoPlayer player;` (no namespace) compile unchanged. The
// declaration lives in `nottawa::` to avoid colliding with anything in the
// engine, but every consumer has historically been in the global namespace.
using IOSurfaceVideoPlayer = nottawa::IOSurfaceVideoPlayer;

#endif /* IOSurfaceVideoPlayer_hpp */
